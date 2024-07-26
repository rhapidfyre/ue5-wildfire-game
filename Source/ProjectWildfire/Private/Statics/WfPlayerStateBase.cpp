// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfPlayerStateBase.h"

#include "AIController.h"
#include "Actors/WfFireStationBase.h"
#include "Actors/WfVoxManager.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/WfGlobalConstants.h"
#include "Lib/WfCalloutData.h"
#include "Logging/StructuredLog.h"
#include "Messages/WfErrorMessages.h"
#include "Net/UnrealNetwork.h"
#include "Statics/WfGameInstanceBase.h"
#include "Statics/WfGameModeBase.h"
#include "Statics/WfGameStateBase.h"
#include "Statics/WfGlobalTags.h"
#include "Vehicles/WfFireApparatusBase.h"
#include "TextToSpeech/Private/Windows/WindowsTextToSpeechFactory.h"


FFirefighterAssignment::FFirefighterAssignment()
	: AssignedVehicle(nullptr), CharacterReference(nullptr)
{
}

FFireApparatusFleet::FFireApparatusFleet()
	: VehicleReference(nullptr), FireStationBase(nullptr), CalloutAssigned(nullptr)
{
}

FFleetPurchaseData::FFleetPurchaseData()
	: DisplayIcon(nullptr), PurchaseValue(0.0f)
{
}

AWfPlayerStateBase::AWfPlayerStateBase()
	: FireStationBase(nullptr),
	  GameModeBase(nullptr),
	  Resources({})
{
}

void AWfPlayerStateBase::NotifyCallout(const AWfCalloutActor* CalloutActor)
{
	if (IsValid(CalloutActor) && OnIncidentReceived.IsBound())
	{
		OnIncidentReceived.Broadcast(CalloutActor);
	}
}

void AWfPlayerStateBase::AssignToCallout(AWfFireApparatusBase* FireApparatus, AWfCalloutActor* CalloutActor)
{
	if (!HasAuthority())
	{
		Server_AssignToCallout(FireApparatus, CalloutActor);
		return;
	}

	if (!IsValid(FireApparatus))
	{
		UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to assign callout - FireApparatus was NULL."
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	if (!IsValid(CalloutActor))
	{
		UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to assign callout - Callout does not exist"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	for (auto& AssignmentData : FleetData)
	{
		if (IsValid(AssignmentData.VehicleReference))
		{
			if (AssignmentData.VehicleReference == FireApparatus)
			{
				// Assign the apparatus to the call
				AssignmentData.CalloutAssigned = CalloutActor;
				Multicast_ApparatusAssignment(FireApparatus, CalloutActor);

				// Assign all personnel on this vehicle to the call
				for (auto& FireFighter : PersonnelData)
				{
					UE_LOGFMT(LogTemp, Display, "PlayerState({NetMode}): Assigned '{CharacterName}' to '{Callout}'"
						, HasAuthority() ? "SRV" : "CLI", FireFighter.CharacterReference->GetCharacterName(), CalloutActor->GetName());

					CalloutActor->AssignUnitToCallout(AssignmentData.VehicleReference, FireFighter.CharacterReference);
				}
				return;
			}
		}
	}
	UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to assign callout. Assigned Apparatus was not found."
		, HasAuthority() ? "SRV" : "CLI");
}

void AWfPlayerStateBase::UnassignFromCallout(AWfFireApparatusBase* FireApparatus)
{
	if (!HasAuthority())
	{
		Server_UnassignFromCallout(FireApparatus);
		return;
	}

	if (!IsValid(FireApparatus))
	{
		UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Failed to unassign callout - FireApparatus was NULL."
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	for (auto& VehicleData : FleetData)
	{
		if (VehicleData.VehicleReference == FireApparatus)
		{
			VehicleData.CalloutAssigned = nullptr;
			Multicast_ApparatusAssignment(FireApparatus, nullptr);
			UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): '{FireApp}' Unassigned"
				, HasAuthority() ? "SRV" : "CLI", FireApparatus->GetName());
			return;
		}
	}
}

void AWfPlayerStateBase::Server_SetFireStation_Implementation(AWfFireStationBase* FireStation)
{
	SetFireStationReference(FireStation);
}

void AWfPlayerStateBase::Client_FireApparatusPurchased_Implementation(const FFireApparatusFleet& NewApparatusData)
{
	if (OnFireApparatusPurchase.IsBound())
	{
		OnFireApparatusPurchase.Broadcast(NewApparatusData.VehicleReference);
	}
}

void AWfPlayerStateBase::OnRep_FleetData_Implementation(const TArray<FFireApparatusFleet>& OldFleetData)
{
    TMap<AWfFireApparatusBase*, AWfFireStationBase*> OldFleetMap;
    TMap<AWfFireApparatusBase*, AWfFireStationBase*> NewFleetMap;
    TSet<AWfFireApparatusBase*> PreviousVehicles;
    TSet<AWfFireApparatusBase*> CurrentVehicles;
    TMap<AWfFireApparatusBase*, AWfFireStationBase*> ChangedAssignments;

    // Populate old fleet map and set of previous vehicles
    for (const auto& OldFleet : OldFleetData)
    {
        if (IsValid(OldFleet.VehicleReference))
        {
            OldFleetMap.Add(OldFleet.VehicleReference, OldFleet.FireStationBase);
            PreviousVehicles.Add(OldFleet.VehicleReference);
        }
    }

    // Populate new fleet map and set of current vehicles
    for (const auto& NewFleet : FleetData)
    {
        if (IsValid(NewFleet.VehicleReference))
        {
            NewFleetMap.Add(NewFleet.VehicleReference, NewFleet.FireStationBase);
            CurrentVehicles.Add(NewFleet.VehicleReference);
        }
    }

    // Detect changed or newly added assignments
    for (const auto& NewAssignment : NewFleetMap)
    {
        AWfFireApparatusBase* Vehicle = NewAssignment.Key;
        AWfFireStationBase* NewStation = NewAssignment.Value;

        if (OldFleetMap.Contains(Vehicle))
        {
            AWfFireStationBase* OldStation = OldFleetMap[Vehicle];
            if (OldStation != NewStation)
            {
                ChangedAssignments.Add(Vehicle, NewStation);
            }
        }
        else
        {
            ChangedAssignments.Add(Vehicle, NewStation);
        }
    }

    // Detect removed vehicles
    for (const auto& OldVehicle : PreviousVehicles)
    {
        if (!CurrentVehicles.Contains(OldVehicle))
        {
            ChangedAssignments.Add(OldVehicle, nullptr);
        }
    }

    // Notify Delegates
    if (OnFleetChanged.IsBound())
    {
        for (const auto& ChangedData : ChangedAssignments)
        {
            FFireApparatusFleet FleetAssignment;
            FleetAssignment.VehicleReference = ChangedData.Key;
            FleetAssignment.FireStationBase  = ChangedData.Value;
        	OnFleetChanged.Broadcast(FleetAssignment);
        	UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): '{VehicleName}' assigned to {StationName}"
				, HasAuthority() ? "SRV" : "CLI", ChangedData.Key->GetApparatusIdentity()
				, IsValid(ChangedData.Value) ? ("Fire Station #" + ChangedData.Value->FireStationNumber) : "NULL");
        }
    }
}

void AWfPlayerStateBase::OnRep_PersonnelData_Implementation(const TArray<FFirefighterAssignment>& OldAssignments)
{
    TMap<AWfFfCharacterBase*, AWfFireApparatusBase*> OldAssignmentMap;
    TMap<AWfFfCharacterBase*, AWfFireApparatusBase*> NewAssignmentMap;
    TSet<AWfFfCharacterBase*> PreviousFirefighters;
    TSet<AWfFfCharacterBase*> CurrentFirefighters;
    TMap<AWfFfCharacterBase*, AWfFireApparatusBase*> ChangedAssignments;

    // Populate old assignments map and set of previous firefighters
    for (const auto& OldAssignment : OldAssignments)
    {
        OldAssignmentMap.Add(OldAssignment.CharacterReference, OldAssignment.AssignedVehicle);
        PreviousFirefighters.Add(OldAssignment.CharacterReference);
    }

    // Populate new assignments map and set of current firefighters
    for (const auto& NewAssignment : PersonnelData)
    {
        if (IsValid(NewAssignment.CharacterReference))
        {
            NewAssignmentMap.Add(NewAssignment.CharacterReference, NewAssignment.AssignedVehicle);
            CurrentFirefighters.Add(NewAssignment.CharacterReference);
        }
    }

    // Detect changed or newly added assignments
    for (const auto& NewAssignment : NewAssignmentMap)
    {
        AWfFfCharacterBase* Character = NewAssignment.Key;
        AWfFireApparatusBase* NewVehicle = NewAssignment.Value;

        if (OldAssignmentMap.Contains(Character))
        {
            AWfFireApparatusBase* OldVehicle = OldAssignmentMap[Character];
            if (OldVehicle != NewVehicle)
            {
                ChangedAssignments.Add(Character, NewVehicle);
            }
        }
        else
        {
            ChangedAssignments.Add(Character, NewVehicle);
        }
    }

    // Detect removed firefighters
    for (const auto& OldFirefighter : PreviousFirefighters)
    {
        if (!CurrentFirefighters.Contains(OldFirefighter))
        {
            ChangedAssignments.Add(OldFirefighter, nullptr);
        }
    }

    // Notify Delegates
    if (OnAssignmentChanged.IsBound())
    {
        for (const auto& ChangedData : ChangedAssignments)
        {
            FFirefighterAssignment FirefighterAssignment;
            FirefighterAssignment.AssignedVehicle = ChangedData.Value;
        	FirefighterAssignment.CharacterReference = ChangedData.Key;
        	UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): '{CharacterName}' assigned to '{VehicleName}'"
				, HasAuthority() ? "SRV" : "CLI", ChangedData.Key->GetCharacterName()
				, IsValid(ChangedData.Value) ? ChangedData.Value->GetApparatusIdentity() : "(unassigned)");
            OnAssignmentChanged.Broadcast(FirefighterAssignment);
        }
    }
}

void AWfPlayerStateBase::Server_SetAssignedApparatus_Implementation(AWfFfCharacterBase* FireCharacter,
	AWfFireApparatusBase* FireApparatus)
{
	SetAssignedApparatus(FireCharacter, FireApparatus);
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

void AWfPlayerStateBase::OnRep_FireStation_Implementation()
{
	UE_LOGFMT(LogTemp, Display, "{PlayerState}({NetMode}) is {NowNoLonger}{FireStationName}"
		, GetName(), HasAuthority() ? "SRV" : "CLI", IsValid(FireStationBase) ? "now the owner of" : "no longer commanding a fire station"
		, IsValid(FireStationBase) ? (" " + FireStationBase->GetName()) : ".");
	if (OnFireStationChanged.IsBound())
	{
		OnFireStationChanged.Broadcast(FireStationBase, IsValid(FireStationBase));
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

AWfFireStationBase* AWfPlayerStateBase::GetFireStationReference() const
{
	return FireStationBase;
}

AWfFireApparatusBase* AWfPlayerStateBase::GetAssignedApparatus(const AWfFfCharacterBase* FireFighter)
{
	if (IsValid(FireFighter))
	{
		for (const auto& FirefighterData : GetAllPersonnel())
		{
			if (FirefighterData.CharacterReference == FireFighter)
				return FirefighterData.AssignedVehicle;
		}
	}
	return nullptr;
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
			UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): No Valid Fire Station"
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
		int NextNumber = 1;
		for (const auto& FleetVehicle : FleetData)
		{
			if (IsValid(FleetVehicle.VehicleReference))
			{
				if (FleetVehicle.VehicleReference->GetApparatusIdentityType() == PurchaseData.ApparatusCallsign
					&& FleetVehicle.VehicleReference->GetApparatusIdentityStation() == FireStationBase->FireStationNumber)
				{
					NextNumber++;
				}
			}
		}

		AWfFireApparatusBase* NewApparatus = GetWorld()->SpawnActorDeferred<AWfFireApparatusBase>
			(SpawnClass, SpawnTransform, OwningFireStation, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (IsValid(NewApparatus))
		{
			NewApparatus->SetIdentities(
				OwningFireStation->FireStationNumber, PurchaseData.ApparatusCallsign.ToString(), NextNumber);

			NewApparatus->FinishSpawning(SpawnTransform);
			FFireApparatusFleet NewFleetData;
			NewFleetData.VehicleReference = NewApparatus;
			NewFleetData.FireStationBase  = FireStationBase;

			FleetData.Add(NewFleetData);

			UE_LOGFMT(LogTemp, Display, "Spawned '{NewCallsign}', owned by Fire Station #{StationNumber}"
				, NewApparatus->GetApparatusIdentity(), NewFleetData.FireStationBase->FireStationNumber);
			if (OnFireApparatusPurchase.IsBound())
			{
				OnFireApparatusPurchase.Broadcast(NewApparatus);
			}
			Client_FireApparatusPurchased(NewFleetData);
		}
		else
		{
			UE_LOGFMT(LogTemp, Error, "Failed to Spawn Apparatus '{NewApparatus}' (Class '{NewClass}')"
				, PurchaseData.DisplayName, PurchaseData.FireApparatusType->GetName());
		}
	}
}

void AWfPlayerStateBase::SetAssignedApparatus(AWfFfCharacterBase* FireCharacter, AWfFireApparatusBase* FireApparatus)
{
	if (!HasAuthority())
	{
		Server_SetAssignedApparatus(FireCharacter, FireApparatus);
		return;
	}

	if (!IsValid(FireCharacter))
	{
		UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): SetAssignedApparatus received NULL 'FireCharacter'"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	FFirefighterAssignment* AssignmentData = nullptr;
	int numPersonnel = 0;
	for (auto& FireFighter : PersonnelData)
	{
		if (IsValid(FireFighter.AssignedVehicle) && IsValid(FireFighter.CharacterReference))
		{
			if (FireFighter.AssignedVehicle == FireApparatus)
			{
				numPersonnel++;
			}
		}
		if (FireFighter.CharacterReference == FireCharacter)
		{
			// If nullifying the fire apparatus, set it to null and return.
			if (!IsValid(FireApparatus))
			{
				AssignmentData->AssignedVehicle = nullptr;
				return;
			}

			// Otherwise, grab this struct and modify it
			AssignmentData = &FireFighter;
		}
	}

	if (numPersonnel > FireApparatus->NumberOfSeats)
	{
		UE_LOGFMT(LogTemp, Error, "PlayerState({NetMode}): Can't assign vehicle '{NewVehicle}'. Max Occupants Reached."
			, HasAuthority() ? "SRV" : "CLI", FireApparatus->GetName());
		return;
	}

	if (AssignmentData)
	{
		if (IsValid(FireApparatus))
			FireApparatus->SetFirefighterAssigned(FireCharacter);
		else if (IsValid(AssignmentData->AssignedVehicle))
			AssignmentData->AssignedVehicle->SetFirefighterAssigned(FireCharacter, false);

		UE_LOGFMT(LogTemp, Display, "PlayerState({NetMode}): Updated Assignment: '{CharacterName}' assigned to '{VehicleName}'"
			, HasAuthority() ? "SRV" : "CLI", FireCharacter->GetCharacterName()
			, IsValid(FireApparatus) ? FireApparatus->GetApparatusIdentity() : "(unassigned)");
	}
	else
	{
		if (IsValid(FireApparatus))
			FireApparatus->SetFirefighterAssigned(FireCharacter);

		FFirefighterAssignment NewAssignment;
		NewAssignment.AssignedVehicle = FireApparatus;
		NewAssignment.CharacterReference = FireCharacter;
		PersonnelData.Add(NewAssignment);

		UE_LOGFMT(LogTemp, Display, "PlayerState({NetMode}): New Assignment: '{CharacterName}' assigned to '{VehicleName}'"
			, HasAuthority() ? "SRV" : "CLI", FireCharacter->GetCharacterName()
			, IsValid(FireApparatus) ? FireApparatus->GetApparatusIdentity() : "(unassigned)");
	}
}

void AWfPlayerStateBase::SetFireStationReference(AWfFireStationBase* FireStation)
{
	if (HasAuthority())
	{
		if (IsValid(FireStation))
		{
			FireStationBase = FireStation;
			UE_LOGFMT(LogTemp, Display, "{PlayerState}({NetMode}) is {NowNoLonger}{FireStationName}"
				, GetName(), HasAuthority() ? "SRV" : "CLI", IsValid(FireStationBase) ? "now the owner of" : "no longer commanding a fire station"
				, IsValid(FireStation) ? (" " + FireStation->GetName()) : ".");
			if (OnFireStationChanged.IsBound())
			{
				OnFireStationChanged.Broadcast(FireStationBase, IsValid(FireStationBase));
			}
		}
	}
	else if (IsValid(FireStation))
	{
		Server_SetFireStation(FireStation);
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

		AAIController* NewController = GetWorld()->SpawnActor<AAIController>
			(UsingAiController, NewTransform);
		NewController->Possess(NewFirefighter);

		FailureContext = "OK";

		RemoveMoney(PaycheckCost);

		FFirefighterAssignment NewAssignment;
		NewAssignment.CharacterReference = NewFirefighter;
		PersonnelData.Add(NewAssignment);;

		if (OnFirefighterHired.IsBound())
			OnFirefighterHired.Broadcast(NewFirefighter);

		AWfGameStateBase* GameStateBase = Cast<AWfGameStateBase>( GetWorld()->GetGameState() );
		if (IsValid(GameStateBase))
		{
			GameStateBase->JobContractRemove(JobContract, false);
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

		FTimerDelegate DateTimeDelegate;
		DateTimeDelegate.BindUObject(this, &AWfPlayerStateBase::SynchronizeTime);
		GetWorldTimerManager().SetTimer(GameDateTimer, DateTimeDelegate, 1.0f, true);
	}
}

void AWfPlayerStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (GameDateTimer.IsValid())
		GameDateTimer.Invalidate();
}

/**
 * \brief Triggered whenever a vehicle's call assignment is changed.
 * \param FireApparatus The vehicle changing assignment
 * \param CalloutActor The callout the vehicle is being assigned to. Nullptr means unassigned.
 */
void AWfPlayerStateBase::Multicast_ApparatusAssignment_Implementation(const AWfFireApparatusBase* FireApparatus,
                                                                      const AWfCalloutActor* CalloutActor)
{
	if (OnApparatusAssignment.IsBound())
		OnApparatusAssignment.Broadcast(FireApparatus, CalloutActor);
}

void AWfPlayerStateBase::Server_PurchaseFireApparatus_Implementation(const FFleetPurchaseData& PurchaseData)
{
	PurchaseFireApparatus(PurchaseData);
}

void AWfPlayerStateBase::HourlyTick(const FDateTime& NewDateTime)
{
	UE_LOGFMT(LogTemp, Display, "HourlyTick()");
	if (HasAuthority())
	{
		// Synchronize time with game mode/server
		GameDateTime = NewDateTime;
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

	DOREPLIFETIME(AWfPlayerStateBase, FireStationBase);

	DOREPLIFETIME_CONDITION(AWfPlayerStateBase, PersonnelData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfPlayerStateBase, FleetData,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfPlayerStateBase, GameDateTime,	COND_OwnerOnly);
}

void AWfPlayerStateBase::Server_AssignToCallout(AWfFireApparatusBase* FireApparatus, AWfCalloutActor* CalloutActor)
{
	AssignToCallout(FireApparatus, CalloutActor);
}

void AWfPlayerStateBase::Server_UnassignFromCallout(AWfFireApparatusBase* FireApparatus)
{
	UnassignFromCallout(FireApparatus);
}

void AWfPlayerStateBase::SynchronizeTime()
{
	if (IsValid(GameModeBase))
	{
		GameDateTime = GameModeBase->GetGameDateTime();
	}
}

void AWfPlayerStateBase::Client_PurchaseError_Implementation(const FText& ErrorReason)
{
	if (OnFireApparatusPurchaseFail.IsBound())
	{
		OnFireApparatusPurchaseFail.Broadcast(ErrorReason);
	}
}
