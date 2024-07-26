// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/WfCalloutData.h"

#include "Actors/WfPropertyActor.h"
#include "Actors/WfVoxManager.h"
#include "Characters/WfFfCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Statics/WfGameModeBase.h"
#include "Statics/WfPlayerStateBase.h"
#include "Vehicles/WfFireApparatusBase.h"


DEFINE_LOG_CATEGORY(LogCallouts);

// Helper function to convert a digit character to its word equivalent
FName DigitToWord(TCHAR Digit)
{
	switch (Digit)
	{
	case '0': return FName("zero");
	case '1': return FName("one");
	case '2': return FName("two");
	case '3': return FName("three");
	case '4': return FName("four");
	case '5': return FName("five");
	case '6': return FName("six");
	case '7': return FName("seven");
	case '8': return FName("eight");
	case '9': return FName("nine");
	default: return FName();
	}
}

// Function to process the address string
void ProcessAddressString(FString AddressString, TArray<FName>& VoxAddresses)
{
	// Split the address string into words
	TArray<FString> AddressComponents;
	AddressString.ParseIntoArrayWS(AddressComponents);

	// Process each component
	for (FString Component : AddressComponents)
	{
		// Check if the component is a number
		for (int32 i = 0; i < Component.Len(); i++)
		{
			if (FChar::IsDigit(Component[i]))
			{
				// Convert each digit to its word equivalent and add to the array
				VoxAddresses.Add(DigitToWord(Component[i]));
			}
			else
			{
				// If it's not a digit, add the word as is
				VoxAddresses.Add(FName(*Component.ToLower()));
				break;
			}
		}
	}
}

FCalloutEquipmentUse::FCalloutEquipmentUse()
	: bNoConsume(false), TotalUsageValue(1.0f)
{
}

FCalloutDataFire::FCalloutDataFire()
	: TaskProgress(0.25f), Difficulty(0.1f), WaterUsage(0.833)
{
}

FCalloutDataMedical::FCalloutDataMedical()
	: TaskProgress(0), RefusalChance(0), Difficulty(0)
{
}

FCalloutAssignment::FCalloutAssignment()
	: AssignedVehicle(nullptr), AssignedCharacter(nullptr)
{
}

FCalloutUnits::FCalloutUnits()
	: QuantityMinimum(0)
{
}

FCallouts::FCallouts()
	: AlertLevel(0),
	  Payment(0),
	  Penalty(0),
	  DifficultyMin(0),
	  DifficultyMax(1),
	  DifficultyVariance(0.2f),
	  PatientsMin(0),
	  PatientsMax(0),
	  RefusalThreshold(0),
	  RefusalChanceMax(0),
	  MinFires(0),
	  MaxFires(0),
	  DeadlineDays(0),
	  DeadlineHours(0),
	  DeadlineMinutes(0)
{
}

FCalloutData::FCalloutData()
	: CalloutActor(nullptr), SecondsToStart(0), PropertyActor(nullptr)
{
	CalloutActor = AWfCalloutActor::StaticClass();
}

FCalloutData::FCalloutData(const FCallouts& NewCallout)
	: SecondsToStart(0), CalloutData(NewCallout), PropertyActor(nullptr)
{
	CalloutActor = AWfCalloutActor::StaticClass();
}

AWfCalloutActor::AWfCalloutActor()
{
}

/**
 * \brief Sets the data this callout will use. Only works when the callout has not yet been started.
 * \param NewCallout The callout data to set. Must be validated before passing.
 * \param SecondsToStart The amount of time players have to assign units and respond
 */
void AWfCalloutActor::SetCalloutData(FCallouts& NewCallout, const float SecondsToStart)
{
	if (bCalloutReady)
	{
		UE_LOGFMT(LogCallouts, Error, "{ThisActor}({NetMode}): Failed to set call data - Callout has already initialized."
			, GetName(), HasAuthority() ? "SRV" : "CLI");
		return;
	}

	FCalloutData NewCallData(NewCallout);
	NewCallData.SecondsToStart = SecondsToStart;

	// Generate the Location
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWfPropertyActor::StaticClass(), AllActors);
	const int RandomIndex = FMath::RandRange(0, AllActors.Num() - 1);
	NewCallData.PropertyActor = Cast<AWfPropertyActor>(AllActors[RandomIndex]);

	if (!IsValid(NewCallData.PropertyActor))
	{
		UE_LOGFMT(LogCallouts, Error, "{ThisActor}({NetMode}): Failed to Setup Callout - No PropertyActor (Location) Found"
			, GetName(), HasAuthority() ? "SRV" : "CLI");
		return;
	}

	// Value & Variable Compliance
	if (NewCallout.PatientsMin < 0) NewCallout.PatientsMin = 0;
	if (NewCallout.PatientsMax < 0) NewCallout.PatientsMax = 0;
	if (NewCallout.MinFires < 0) NewCallout.MinFires = 0;
	if (NewCallout.MaxFires < 0) NewCallout.MaxFires = 0;

	// The call must have at least 1 patient or 1 fire
	if (NewCallout.PatientsMax == 0 && NewCallout.MaxFires == 0)
	{
		UE_LOGFMT(LogCallouts, Error, "{ThisActor}({NetMode}): Failed to Setup Callout - Call Type '{CallType}' has no fire or patient data"
			, GetName(), HasAuthority() ? "SRV" : "CLI", NewCallout.DisplayName);
		return;
	}

	// Determine Difficulty
	NewCallout.DifficultyMin = FMath::Clamp(NewCallout.DifficultyMin, 0.0f, 1.0f);
	NewCallout.DifficultyMax = FMath::Clamp(NewCallout.DifficultyMax, NewCallout.DifficultyMin, 1.0f);
	NewCallout.DifficultyVariance = FMath::Clamp(NewCallout.DifficultyVariance, 0.0f, 1.0f);
	float Difficulty = FMath::RandRange(NewCallout.DifficultyMin, NewCallout.DifficultyMax);

	// Difficulty determines how long the patient takes to treat, not the number of patients
	int NumberOfPatients = NewCallout.PatientsMax;
	if (NewCallout.PatientsMax > 0)
	{
		if (NewCallout.PatientsMin != NewCallout.PatientsMax)
			NumberOfPatients = FMath::RandRange(NewCallout.PatientsMin, NewCallout.PatientsMax);

		for (int i = 0; i < NumberOfPatients; ++i)
		{
			const float VarianceRange = FMath::RandRange(NewCallout.DifficultyVariance * -1, NewCallout.DifficultyVariance);

			FCalloutDataMedical NewPatient;
			NewPatient.EquipmentUsage = NewCallout.MedicalEquipment;

			// Patient Difficulty
			NewPatient.Difficulty =	FMath::Clamp(Difficulty + VarianceRange, 0.0f, 1.0f);
			const float DiffThreshold = FMath::Clamp(NewPatient.Difficulty, 0.0f, NewCallout.RefusalThreshold);
			if (DiffThreshold > 0.0f)
				NewPatient.RefusalChance  = NewCallout.RefusalChanceMax * (1.0f - DiffThreshold / NewCallout.RefusalThreshold);
			else
				NewPatient.RefusalChance = 0.0f;
		}
	}

	// The number of fires the call will begin with before spread
	// Difficulty determines how quickly the fire regenerates, spreads, and how hard it is to extinguish
	int NumberOfFires = NewCallout.MaxFires;
	if (NewCallout.MaxFires > 0)
	{
		if (NewCallout.PatientsMin != NewCallout.PatientsMax)
			NumberOfFires = FMath::RandRange(NewCallout.MinFires, NewCallout.MaxFires);

		for (int i = 0; i < NumberOfFires; ++i)
		{
			const float VarianceRange = FMath::RandRange(NewCallout.DifficultyVariance * -1, NewCallout.DifficultyVariance);

			FCalloutDataFire NewFire;
			NewFire.EquipmentUsage	= NewCallout.FireEquipment;
			NewFire.Difficulty		= FMath::Clamp(Difficulty + VarianceRange, 0.0f, 1.0f);

			// Modify water usage based on difficulty
			NewFire.WaterUsage	   += NewFire.Difficulty * NewFire.WaterUsage;
		}
	}

	// Determine completion time and vox callouts

	if (NewCallout.VoxPhrases.IsNone())
	{
		if (NumberOfFires > 0)
			NewCallout.VoxPhrases = "structure_fire";
		else if (NumberOfFires > 0)
			NewCallout.VoxPhrases = "structure_fire";
		else
			NewCallout.VoxPhrases = "medical";
	}

	const FDateTime CurrentGdt  = GetGameDateTime();
	const FTimespan AddTimespan = FTimespan(NewCallout.DeadlineDays, NewCallout.DeadlineHours, NewCallout.DeadlineMinutes, 0);
	NewCallData.ResolutionDeadline	= CurrentGdt + AddTimespan;
	NewCallData.ServerTimeStart		= CurrentGdt;
	CalloutData = NewCallData;
}

/**
 * \brief Starts the callout, if the data is valid.
 *	Initiates the dispatch process, and creates the response prompt for all connected players.
 * \return True if the callout is valid and started. False otherwise.
 */
bool AWfCalloutActor::StartCallout()
{

	// The callout expiration is after the current date and time
	const AWfGameModeBase* GameMode = Cast<AWfGameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		FDateTime GameDateTime = GameMode->GetGameDateTime();
		if (CalloutData.ResolutionDeadline > GameDateTime)
		{
			DispatchInitial();

			// Start the expiration detection
			FTimerDelegate ExpirationDelegate;
			ExpirationDelegate.BindUObject(this, &AWfCalloutActor::CheckCalloutExpired);
			GetWorldTimerManager().SetTimer(ExpirationTimer, ExpirationDelegate, 1.0f, true, 1.0f);

			return true;
		}

		// Deadline has Passed
		UE_LOGFMT(LogCallouts, Error, "{ThisName}({NetMode}): Callout Resolution Deadline ({Deadline}) was sooner than current game time ({GameTime})"
			, GetName(), HasAuthority() ? "SRV" : "CLI", CalloutData.ResolutionDeadline.ToString(), GameDateTime.ToString());
		return false;
	}

	// Executed on Client, or Gamemode Invalid
	UE_LOGFMT(LogCallouts, Error, "{ThisName}({NetMode}): Either 'Startcallout' was executed on a client, or AWfGameMode was Invalid."
		, GetName(), HasAuthority() ? "SRV" : "CLI");
	return false;
}

FDateTime AWfCalloutActor::GetGameDateTime() const
{
	const AWfGameModeBase* GameMode = Cast<AWfGameModeBase>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		FDateTime GameDateTime = GameMode->GetGameDateTime();
		return GameDateTime;
	}
	return FDateTime::UtcNow();
}

void AWfCalloutActor::AssignUnitToCallout(AWfFireApparatusBase* FireVehicle, AWfFfCharacterBase* FireFighter)
{
	if (IsValid(FireVehicle) && IsValid(FireFighter))
	{
		FCalloutAssignment CalloutAssignment;
		CalloutAssignment.AssignedCharacter = FireFighter;
		CalloutAssignment.AssignedVehicle	= FireVehicle;
		for (const auto& AssignedUnit : AssignedUnits)
		{
			bool bSameVehicle =		AssignedUnit.AssignedVehicle   == CalloutAssignment.AssignedVehicle;
			bool bSameCharacter =	AssignedUnit.AssignedCharacter == CalloutAssignment.AssignedCharacter;

			if (!bSameVehicle || (bSameVehicle && !bSameCharacter))
			{
				AssignedUnits.Add(CalloutAssignment);
				UE_LOGFMT(LogCallouts, Error, "{ThisName}({NetMode}): {CharName} ({AppName}) has been assigned to this callout."
					, GetName(), HasAuthority() ? "SRV" : "CLI", CalloutAssignment.AssignedCharacter->GetName(), CalloutAssignment.AssignedVehicle->GetName());
			}
		}
	}
}

void AWfCalloutActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWfCalloutActor, CalloutData);
}

void AWfCalloutActor::ApplyExpiredPenalty()
{
	if (!HasAuthority())
		return;
}

/**
 * \brief Performs the initial callout, doing radio traffic and enabling dispatching for players/units
 */
void AWfCalloutActor::DispatchInitial()
{
	if (!HasAuthority())
		return;

	bCalloutReady = true;

	// Start the callout timer (progression, value detection, etc)
	FTimerDelegate CalloutDelegate;
	CalloutDelegate.BindUObject(this, &AWfCalloutActor::CalloutTick);
	GetWorldTimerManager().SetTimer(CalloutTimer, CalloutDelegate, 1.0f, true, 1.0f);

	Multicast_DispatchPreAlert();

	TArray<FName> VoxPhrases = {"start_tx", "alert3", "_blank", "golden_crest", "_blank"};
	VoxPhrases.Add(CalloutData.CalloutData.VoxPhrases); // Call Type
	ProcessAddressString(CalloutData.PropertyActor->GetStreetAddressAsString(), VoxPhrases);

	VoxPhrases.Add("_blank");
	VoxPhrases.Add("stop_tx");

	// Get the Vox Singleton and send the pre-alert
	AWfVoxManager* VoxManager = AWfVoxManager::GetInstance(GetWorld());
	if (IsValid(VoxManager))
	{
		VoxManager->SpeakSentence(VoxPhrases, false, true);
	}
}

/**
 * \brief Performs the full dispatch of the incident, destroying the callout if no-one was assigned to it.
 */
void AWfCalloutActor::DispatchCallout()
{
	if (!HasAuthority())
		return;

	Multicast_DispatchCallout();
}

void AWfCalloutActor::CheckCalloutExpired()
{
	FDateTime GameDateTime = GetGameDateTime();
	if (CalloutData.ResolutionDeadline < GameDateTime)
	{
		if (ExpirationTimer.IsValid())
			ExpirationTimer.Invalidate();

		if (CalloutTimer.IsValid())
			CalloutTimer.Invalidate();

		UE_LOGFMT(LogCallouts, Warning, "{ThisName}({NetMode}): This callout has passed the deadline and has expired."
			, GetName(), HasAuthority() ? "SRV" : "CLI");

		Destroy();
	}
}

void AWfCalloutActor::CalloutTick()
{
	// TODO
}


/**
 *		NETWORKING & REPLICATION
 */


/**
 * \brief Notifies delegates (such as the vox/dispatch and ui systems) that a call is new and has started
 */
void AWfCalloutActor::Multicast_DispatchPreAlert_Implementation()
{
	if (OnCalloutDispatchInitial.IsBound())
		OnCalloutDispatchInitial.Broadcast(this);

	// Notify all players that there is a new callout
	UWorld* WorldReference = GetWorld();

	const ULocalPlayer* LocalPlayer = WorldReference->GetFirstLocalPlayerFromController();
	if (!IsValid(LocalPlayer)) return;

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(WorldReference);
	if (!IsValid(PlayerController)) return;

	// Notify the PlayerState of the new callout
	AWfPlayerStateBase* PlayerState = Cast<AWfPlayerStateBase>(PlayerController->PlayerState);
	if (IsValid(PlayerState))
	{
		PlayerState->NotifyCallout(this);
	}
}

/**
 * \brief Notifies delegates (such as the vox/dispatch and ui systems) that the call has been dispatched
 */
void AWfCalloutActor::Multicast_DispatchCallout_Implementation()
{
	if (OnCalloutDispatchFull.IsBound())
		OnCalloutDispatchFull.Broadcast(this);
}
