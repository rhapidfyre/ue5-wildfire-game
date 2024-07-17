// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfPlayerStateBase.h"

#include "AIController.h"
#include "Actors/WfFireStationBase.h"
#include "Characters/WfCharacterTags.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Saves/WfPlayerSave.h"
#include "Statics/WfGameInstanceBase.h"
#include "Statics/WfGameModeBase.h"
#include "Statics/WfGameStateBase.h"
#include "Statics/WfGlobalTags.h"

AWfPlayerStateBase::AWfPlayerStateBase()
	: FireStationBase(nullptr),
	  GameModeBase(nullptr),
	  Resources({})
{
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

AWfFireStationBase* AWfPlayerStateBase::GetFireStationReference()
{
	if (!IsValid(FireStationBase))
	{
		TArray<AActor*> FireStations;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWfFireStationBase::StaticClass(), FireStations);
		for (const auto& StationActor : FireStations)
		{
			AWfFireStationBase* FireStation = Cast<AWfFireStationBase>(StationActor);
			if (FireStation->GetOwner() == this)
			{
				FireStationBase = FireStation;
			}
		}
	}
	return FireStationBase;
}

void AWfPlayerStateBase::SetFireStationReference(AWfFireStationBase* FireStation)
{
	if (IsValid(FireStation))
	{
		FireStationBase = FireStation;
	}
}

AWfFfCharacterBase* AWfPlayerStateBase::AcceptJobContract(const FJobContractData& JobContract, FString& FailureContext)
{
	if (!HasAuthority())
	{
		Server_AcceptContract(JobContract);
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


		AWfFfCharacterBase* NewFirefighter = GetWorld()->SpawnActorDeferred<AWfFfCharacterBase>
			(AWfFfCharacterBase::StaticClass(), NewTransform);

		NewFirefighter->SetJobContract(JobContract.ContractId, JobContract.UserIndex);
		NewFirefighter->SetCharacterRole(JobContract.CharacterRole);
		NewFirefighter->SetCharacterRace(JobContract.CharacterRace);
		NewFirefighter->SetCharacterAge(JobContract.CharacterAge);
		NewFirefighter->SetCharacterGender(JobContract.CharacterGender);
		NewFirefighter->SetHourlyRate(JobContract.HourlyRate);


		NewFirefighter->SetCharacterName(
			{JobContract.CharacterNameFirst, JobContract.CharacterNameMiddle, JobContract.CharacterNameLast});
		NewFirefighter->FinishSpawning(NewTransform);

		AAIController* NewController = GetWorld()->SpawnActor<AAIController>
			(UsingAiController, NewTransform);
		NewController->Possess(NewFirefighter);

		FailureContext = "OK";

		RemoveMoney(PaycheckCost);

		AllPersonnel.Add(NewFirefighter);
		if (OnFirefighterHired.IsBound())
			OnFirefighterHired.Broadcast(NewFirefighter);

		AWfGameStateBase* GameStateBase = Cast<AWfGameStateBase>( GetWorld()->GetGameState() );
		if (IsValid(GameStateBase))
		{
			GameStateBase->JobContractRemove(JobContract);
		}

		UE_LOGFMT(LogTemp, Display, "AcceptJobContract({NetMode}): Successfully Hired '{CharacterName}'!"
			, HasAuthority() ? "SRV" : "CLI", NewFirefighter->GetCharacterName());
		return NewFirefighter;
	}
	UE_LOGFMT(LogTemp, Display, "AcceptJobContract({NetMode}): Not enough money to hire '{CharacterName}' - Have ${MyMony}, but costs ${Paycheck}"
		, HasAuthority() ? "SRV" : "CLI",
		JobContract.CharacterNameFirst + JobContract.CharacterNameMiddle + JobContract.CharacterNameLast,
		GetMoney(), PaycheckCost);
	FailureContext = "Insufficient Funds";
	return nullptr;
}

void AWfPlayerStateBase::Server_AcceptContract_Implementation(const FJobContractData& JobContract)
{
	if (!HasAuthority()) { return; }

	FString FailureContext;
	const AWfFfCharacterBase* NewFirefighter = AcceptJobContract(JobContract, FailureContext);
	if (IsValid(NewFirefighter))
	{
		if (OnFirefighterHired.IsBound())
			OnFirefighterHired.Broadcast(NewFirefighter);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "Failed to create firefighter actor from Job Contract '{JobContract}'."
			, JobContract.CharacterNameFirst + JobContract.CharacterNameMiddle + JobContract.CharacterNameLast);
	}
}

FGameplayTagContainer AWfPlayerStateBase::GetAllResourceTags() const
{
	TArray<FGameplayTag> TagArray;
	Resources.GenerateKeyArray(TagArray);
	return FGameplayTagContainer::CreateFromArray(TagArray);
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
	SetupInitialResourceValues();
	if (HasAuthority())
	{
		GameModeBase = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
		if (IsValid(GameModeBase))
		{
			GameModeBase->OnGameHourlyTick.AddDynamic(this, &AWfPlayerStateBase::HourlyTick);
		}
	}
}

void AWfPlayerStateBase::HourlyTick(const FDateTime& NewDateTime)
{
	UE_LOGFMT(LogTemp, Display, "HourlyTick()");
	if (HasAuthority())
	{

	}
}

void AWfPlayerStateBase::Multicast_FirefighterHired_Implementation(const AWfFfCharacterBase* NewFirefighter)
{
	if (OnFirefighterHired.IsBound())
		OnFirefighterHired.Broadcast(NewFirefighter);
}

void AWfPlayerStateBase::Multicast_FirefighterFired_Implementation(const AWfFfCharacterBase* OldFirefighter)
{
	if (OnFirefighterFired.IsBound())
		OnFirefighterFired.Broadcast(OldFirefighter);
}

void AWfPlayerStateBase::Multicast_FirefighterDeath_Implementation(const AWfFfCharacterBase* DeadFirefighter)
{
	if (OnFirefighterDeath.IsBound())
		OnFirefighterDeath.Broadcast(DeadFirefighter);
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
	DOREPLIFETIME_CONDITION(AWfPlayerStateBase, AllPersonnel, COND_OwnerOnly);
}
