// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfPlayerStateBase.h"

#include "AIController.h"
#include "Actors/GameManager.h"
#include "Actors/WfFireStationBase.h"
#include "Actors/WfVoxManager.h"
#include "Lib/WfGlobalConstants.h"
#include "Logging/StructuredLog.h"
#include "Statics/WfGameInstanceBase.h"
#include "Statics/WfGameStateBase.h"
#include "Statics/WfGlobalTags.h"
#include "Vehicles/WfFireApparatusBase.h"

FFleetPurchaseData::FFleetPurchaseData()
	: DisplayIcon(nullptr), PurchaseValue(0.0f)
{
}

AWfPlayerStateBase::AWfPlayerStateBase()
	: FireStationBase(nullptr),
	  Resources({})
{
}

void AWfPlayerStateBase::SetupListeners()
{
	AGameManager* GameManager = AGameManager::GetInstance(GetWorld());
	if (!IsValid(GameManager))
	{
		FTimerHandle GameManagerTimer;
		GetWorldTimerManager().SetTimer(GameManagerTimer,
			this, &AWfPlayerStateBase::SetupListeners, 1.0f, false);
		return;
	}
	//GameManager->OnFirefighterHired.AddDynamic(this, &AWfPlayerStateBase::)
}

void AWfPlayerStateBase::SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue)
{
	if (ResourceTag.IsValid())
	{
		const float OldValue = Resources.Contains(ResourceTag) ? *Resources.Find(ResourceTag) : 0.0f;
		Resources.Add(ResourceTag, NewValue);
		if (OnResourceUpdated.IsBound())
		{
			OnResourceUpdated.Broadcast(ResourceTag, OldValue, NewValue);
			UE_LOGFMT(LogTemp, Display,
				"PlayerState({NetMode}): Resource '{ResourceTag}' Updated ({OldValue} -> {NewValue})",
				HasAuthority() ? "SRV" : "CLI", ResourceTag.ToString(), OldValue, NewValue);
		}
	}
}

void AWfPlayerStateBase::OnRep_FireStationBase_Implementation(const AWfFireStationBase* OldFireStation)
{
	if (OnFireStationChanged.IsBound())
		OnFireStationChanged.Broadcast(IsValid(FireStationBase) ? FireStationBase : OldFireStation, IsValid(FireStationBase));
}

void AWfPlayerStateBase::SetMoney(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetKilowattUsage(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetOxygenReserve(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetWaterStorage(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

float AWfPlayerStateBase::AddMoney(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Money.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddKilowattUsage(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Power.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddOxygenReserve(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Oxygen.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddWaterStorage(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Water.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}


float AWfPlayerStateBase::GetResourceValue(const FGameplayTag& ResourceTag) const
{
	if (ResourceTag.IsValid())
	{
		if (Resources.Contains(ResourceTag))
		{
			const float ResourceValue = Resources[ResourceTag] * 100.0f;
			return FMath::RoundToInt(ResourceValue)/100.0f;
		}
	}
	return 0.0f;
}

void AWfPlayerStateBase::SetFireStationReference(AWfFireStationBase* FireStation)
{
	if (!HasAuthority())
	{
		Server_SetFireStationReference(FireStation);
		return;
	}
	if (FireStationBase == FireStation)
	{
		UE_LOGFMT(LogTemp, Display, "No Change in Fire Station. SetFireStationReference() Ignored.");
		return;
	}
	FireStationBase = FireStation;
}

void AWfPlayerStateBase::PurchaseFireApparatus(const FFleetPurchaseData& PurchaseData)
{
	if (PurchaseData.FireApparatusType)
	{
		if (!HasAuthority())
		{
			Server_PurchaseFireApparatus(PurchaseData);
			return;
		}

		if (!IsValid(FireStationBase))
		{
			UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Attempted to purchase apparatus without a Fire Station."
				, HasAuthority() ? "SRV" : "CLI");
			return;
		}

		// Determine the fire station's available parking spots
		FTransform SpawnTransform(FVector(0.0f));
		if (IsValid(FireStationBase))
		{
			UParkingSpotComponent* ParkingSpot = nullptr;
			for (const auto& ParkSpot : FireStationBase->GetParkingSpots())
			{
				if (!IsValid(ParkSpot->AssignedVehicle))
				{
					ParkingSpot = ParkSpot;
					break;
				}
			}
			if (!IsValid(ParkingSpot))
			{
				UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): No Parking Spots Available", HasAuthority() ? "SRV" : "CLI");
				Client_PurchaseError(WfError::GError_No_Park_Spots);
				if (OnFireApparatusPurchaseFail.IsBound())
					OnFireApparatusPurchaseFail.Broadcast(WfError::GError_No_Park_Spots);
				return;
			}

			FVector SpawnLocation(ParkingSpot->GetComponentLocation());
			FQuat   SpawnRotation(ParkingSpot->GetComponentQuat());
			SpawnTransform.SetLocation(SpawnLocation);
			SpawnTransform.SetRotation(SpawnRotation);
		}

		if (GetMoney() < PurchaseData.PurchaseValue)
		{
			Client_PurchaseError(WfError::GError_Not_Enough_Money);
			return;
		}
		RemoveMoney(PurchaseData.PurchaseValue);

		AWfFireStationBase* OwningFireStation = GetFireStationReference();
		TSubclassOf<AWfFireApparatusBase> SpawnClass = PurchaseData.FireApparatusType;

		// Determine the next vehicle unique number in sequence
		AGameManager* GameManager = AGameManager::GetInstance(GetWorld());
		if (!IsValid(GameManager))
		{
			UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to purchase fire apparatus - Game Manager not Found", HasAuthority() ? "SRV" : "CLI");
			return;
		}

		AWfFireApparatusBase* NewApparatus = GetWorld()->SpawnActorDeferred<AWfFireApparatusBase>
			(SpawnClass, SpawnTransform, OwningFireStation, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (IsValid(NewApparatus))
		{
			NewApparatus->SetOwner( GetPlayerController() );
			NewApparatus->FinishSpawning(SpawnTransform);
			GameManager->AssignFireApparatusToFireStation(NewApparatus, FireStationBase);
		}
		else
		{
			UE_LOGFMT(LogTemp, Error, "Failed to Spawn Apparatus '{NewApparatus}' (Class '{NewClass}')"
				, PurchaseData.DisplayName, PurchaseData.FireApparatusType->GetName());
		}
	}
}

AWfFfCharacterBase* AWfPlayerStateBase::AcceptJobContract(const FJobContractData& JobContract, FString& FailureContext)
{
	if (!HasAuthority())
	{
		Server_AcceptJobContract(JobContract);
		FailureContext = "Unauthorized";
		return nullptr;
	}

	// Ensure player can afford first pay period
	float PaycheckCost = JobContract.HourlyRate * 80;

	if (GetMoney() >= PaycheckCost)
	{
		FTransform NewTransform(FVector(0.f));
		if (IsValid(FireStationBase))
			NewTransform = FireStationBase->SpawnPoint->GetComponentTransform();

		AGameManager* GameManager = AGameManager::GetInstance(GetWorld());
		if (!IsValid(GameManager))
		{
			UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to accept job contract - Game Manager not Found", HasAuthority() ? "SRV" : "CLI");
			return nullptr;
		}

		AWfFfCharacterBase* NewFirefighter = GetWorld()->SpawnActorDeferred<AWfFfCharacterBase>
			(AWfFfCharacterBase::StaticClass(), NewTransform);

		NewFirefighter->SetOwner( GetPlayerController() );
		NewFirefighter->SetJobContract(JobContract.ContractId, JobContract.UserIndex);
		NewFirefighter->SetCharacterRole(JobContract.CharacterRole);
		NewFirefighter->SetCharacterRace(JobContract.CharacterRace);
		NewFirefighter->SetCharacterAge(JobContract.CharacterAge);
		NewFirefighter->SetCharacterGender(JobContract.CharacterGender);
		NewFirefighter->SetHourlyRate(JobContract.HourlyRate);

		NewFirefighter->SetCharacterName(
			{JobContract.CharacterNameFirst, JobContract.CharacterNameMiddle, JobContract.CharacterNameLast});
		NewFirefighter->FinishSpawning(NewTransform);
		GameManager->AssignFirefighterToFireStation(NewFirefighter, FireStationBase);

		RemoveMoney(PaycheckCost);

		AWfGameStateBase* GameStateBase = Cast<AWfGameStateBase>( GetWorld()->GetGameState() );
		if (IsValid(GameStateBase))
		{
			GameStateBase->JobContractRemove(JobContract, false);
		}

		AAIController* NewController = GetWorld()->SpawnActor<AAIController>(UsingAiController, NewTransform);
		NewController->Possess(NewFirefighter);

		FailureContext = "OK";
		return NewFirefighter;
	}
	UE_LOGFMT(LogTemp, Warning, "AcceptJobContract({NetMode}): Not enough money to hire '{CharacterName}' - Have ${MyMony}, but costs ${Paycheck}"
		, HasAuthority() ? "SRV" : "CLI",
		JobContract.CharacterNameFirst + JobContract.CharacterNameMiddle + JobContract.CharacterNameLast,
		GetMoney(), PaycheckCost);
	FailureContext = "Insufficient Funds";
	return nullptr;
}

void AWfPlayerStateBase::Server_AcceptJobContract_Implementation(const FJobContractData& JobContract)
{
	if (!HasAuthority()) { return; }

	FString FailureContext;
	AcceptJobContract(JobContract, FailureContext);
}

FGameplayTagContainer AWfPlayerStateBase::GetAllResourceTags() const
{
	TArray<FGameplayTag> TagArray;
	Resources.GenerateKeyArray(TagArray);
	return FGameplayTagContainer::CreateFromArray(TagArray);
}

AWfFireStationBase* AWfPlayerStateBase::GetFireStationReference() const
{
	return FireStationBase;
}

float AWfPlayerStateBase::GetMoney() const
{
	return GetResourceValue(TAG_Resource_Money.GetTag());
}

float AWfPlayerStateBase::GetKilowattUsage() const
{
	return GetResourceValue(TAG_Resource_Power.GetTag());
}

float AWfPlayerStateBase::GetOxygenReserve() const
{
	return GetResourceValue(TAG_Resource_Oxygen.GetTag());
}

float AWfPlayerStateBase::GetWaterStorage() const
{
	return GetResourceValue(TAG_Resource_Water.GetTag());
}

void AWfPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();
	SetupListeners();
	SetupInitialResourceValues();
}

void AWfPlayerStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AWfPlayerStateBase::Server_SetFireStationReference_Implementation(AWfFireStationBase* FireStation)
{
	// TODO - Make more secure later
	SetFireStationReference(FireStation);
}

void AWfPlayerStateBase::Server_PurchaseFireApparatus_Implementation(const FFleetPurchaseData& PurchaseData)
{
	PurchaseFireApparatus(PurchaseData);
}

void AWfPlayerStateBase::SetupInitialResourceValues()
{
	UWfGameInstanceBase* GameInstanceBase = Cast<UWfGameInstanceBase>(GetGameInstance());
	if (!IsValid(GameInstanceBase))
		return;

	for (const auto& KeyPair : GameInstanceBase->StartingResources)
	{
		Resources.Add(KeyPair.Key, KeyPair.Value);
		if (OnResourceUpdated.IsBound())
		{
			OnResourceUpdated.Broadcast(KeyPair.Key, 0.0f, KeyPair.Value);
		}
	}
}

void AWfPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AWfPlayerStateBase::Client_PurchaseError_Implementation(const FText& ErrorReason)
{
	if (OnFireApparatusPurchaseFail.IsBound())
	{
		OnFireApparatusPurchaseFail.Broadcast(ErrorReason);
	}
}
