// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/CalloutsManager.h"

#include "EngineUtils.h"
#include "Actors/WfPropertyActor.h"
#include "Lib/WfCalloutData.h"
#include "Logging/StructuredLog.h"
#include "Statics/WfGameModeBase.h"


ACalloutsManager* ACalloutsManager::Instance = nullptr;

ACalloutsManager::ACalloutsManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACalloutsManager::BeginPlay()
{
	Super::BeginPlay();

	if (Instance == nullptr)
	{
		Instance = this;

		//GetWorldTimerManager().SetTimer(CalloutTimerHandle, this, &ACalloutsManager::GenerateCallout, 300.0f, true);
	}
	else
	{
		Destroy();
	}
}

void ACalloutsManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (Instance)
	{
		Instance = nullptr;
	}
}

/**
 * \brief Intermediary function to allow blueprints to interact with generated callouts prior to creation
		  Internally calls CreateCallout - Do logic BEFORE calling parent/Super
 * \param CalloutType The FName of the callout as found in the Data Table
 * \param CalloutData Data for callout creation. Can be passed in with custom data. Will be modified during execution.
 *					  It is perfectly acceptable to pass an empty uninitialized FCalloutData struct.
 */
void ACalloutsManager::NewCallout_Implementation(const FName& CalloutType, FCalloutData& CalloutData)
{
	NewCallout(CalloutType, CalloutData);
}

void ACalloutsManager::ForceCallout(const FName& CalloutType)
{
	FCallouts CalloutData = GetCalloutDataRow(CalloutType);
	UE_LOGFMT(LogCallouts, Warning, "ACalloutsManager({NetMode}): GenerateCallout() Forcing Callout '{CallName}'"
		, HasAuthority() ? "SRV" : "CLI", CalloutData.DisplayName);
	FCalloutData NewCalloutData(CalloutData);
	CreateCallout(CalloutType, NewCalloutData);
}

ACalloutsManager* ACalloutsManager::GetInstance(UObject* WorldContext)
{
	if (Instance == nullptr)
	{
		if (WorldContext)
		{
			if (UWorld* World = WorldContext->GetWorld())
			{
				for (TActorIterator<ACalloutsManager> It(World); It; ++It)
				{
					return *It;
				}

				// If no instance exists, spawn one (optional)
				Instance = World->SpawnActor<ACalloutsManager>();
			}
		}
	}
	return Instance;
}

void ACalloutsManager::GenerateCallout()
{
	AWfGameModeBase* GameMode = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameMode))
	{
		if (UDataTable* dt = GameMode->CalloutsTable)
		{
			TArray<FName> RowNames = dt->GetRowNames();
			FName RandomRowName = RowNames[FMath::RandRange(0, RowNames.Num() - 1)];
			FCallouts CalloutData = GetCalloutDataRow(RandomRowName);

			UE_LOGFMT(LogCallouts, Display, "ACalloutsManager({NetMode}): GenerateCallout() Generated Callout '{CallName}'"
			, HasAuthority() ? "SRV" : "CLI", CalloutData.DisplayName);

			FCalloutData NewCalloutData(CalloutData);
			CreateCallout(RandomRowName, NewCalloutData);
			return;
		}
	}
	UE_LOGFMT(LogCallouts, Error, "ACalloutsManager({NetMode}): GenerateCallout() Failed to Execute"
		, HasAuthority() ? "SRV" : "CLI");
}

void ACalloutsManager::Multicast_CalloutExpired(const FCalloutData& CalloutData)
{
	if (OnCalloutExpired.IsBound())
	{
		OnCalloutExpired.Broadcast(CalloutData);
	}
}

void ACalloutsManager::Server_ForceCallout_Implementation(const FName& CalloutType)
{
	ForceCallout(CalloutType);
}

/**
 * \brief Creates a new callout, spawning the actor and all necessary actors/data to do so.
 * Preform all logic before calling parent/super. This completes all logic in creating the callout.
 * \param CalloutType The Data Table Row Name of the callout to be created. It has been validated by this point.
 * \param CalloutData Data for callout creation. Can be passed in with custom data. Will be modified during execution.
 *					  It is perfectly acceptable to pass an empty uninitialized FCalloutData struct.
 */
void ACalloutsManager::CreateCallout(const FName& CalloutType, FCalloutData& CalloutData)
{
	if (!HasAuthority())
	{
		CalloutData.CallLogging.Add("Cannot Execute on Client - Must Run on Server.");
		return;
	}

	// Try to spawn the callout actor at the same location as the call itself, just for organization.
	FTransform SpawnTransform(FVector(0.0f));
	if (IsValid(CalloutData.PropertyActor))
	{
		SpawnTransform.SetLocation(CalloutData.PropertyActor->GetActorLocation());
		SpawnTransform.SetRotation(CalloutData.PropertyActor->GetActorQuat());
	}

	const AWfGameModeBase* GameMode = Cast<AWfGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameMode))
	{
		CalloutData.CallLogging.Add("Invalid GameMode");
		UE_LOGFMT(LogCallouts, Display, "ACalloutsManager({NetMode}): Failed to Generate Callout - Invalid GameMode"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	UDataTable* dt = GameMode->CalloutsTable;
	if (!IsValid(dt))
	{
		CalloutData.CallLogging.Add("Invalid DataTable");
		UE_LOGFMT(LogCallouts, Display, "ACalloutsManager({NetMode}): Failed to Generate Callout - Callout Data Table was NOT SET in GameMode!"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	FString ContextString;
	FCallouts* CalloutRowData = dt->FindRow<FCallouts>(CalloutType, ContextString);
	if (CalloutRowData == nullptr)
	{
		CalloutData.CallLogging.Add("Invalid DataTable");
		UE_LOGFMT(LogCallouts, Display, "ACalloutsManager({NetMode}): Callout Row '{RowName}' NOT FOUND in Callouts Data Table!"
			, HasAuthority() ? "SRV" : "CLI", CalloutType);
		return;
	}

	AWfCalloutActor* CalloutActor = GetWorld()->SpawnActorDeferred<AWfCalloutActor>
		(CalloutData.CalloutActor, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (IsValid(CalloutActor))
	{
		CalloutActor->SetCalloutData(*CalloutRowData, SecondsToRespond);
		CalloutActor->FinishSpawning(SpawnTransform);
		CalloutActor->StartCallout();
	}
}

FCallouts ACalloutsManager::GetCalloutDataRow(const FName& CalloutType) const
{
	AWfGameModeBase* GameMode = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameMode))
	{
		if (UDataTable* dt = GameMode->CalloutsTable)
		{
			if (const FCallouts* VoxDataRow = dt->FindRow<FCallouts>(CalloutType, TEXT("")))
			{
				return *VoxDataRow;
			}
		}
	}
	return {};
}
