// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/CalloutsManager.h"

#include "EngineUtils.h"
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

void ACalloutsManager::ForceCallout(const FName& CalloutType)
{
	FCallouts CalloutData = GetCalloutData(CalloutType);
	UE_LOGFMT(LogTemp, Warning, "ACalloutsManager({NetMode}): GenerateCallout() Forcing Callout '{CallName}'"
		, HasAuthority() ? "SRV" : "CLI", CalloutData.DisplayName);
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
			FCallouts CalloutData = GetCalloutData(RandomRowName);
			UE_LOGFMT(LogTemp, Display, "ACalloutsManager({NetMode}): GenerateCallout() Generated Callout '{CallName}'"
				, HasAuthority() ? "SRV" : "CLI", CalloutData.DisplayName);
			return;
		}
	}
	UE_LOGFMT(LogTemp, Error, "ACalloutsManager({NetMode}): GenerateCallout() Failed to Execute"
		, HasAuthority() ? "SRV" : "CLI");
}

void ACalloutsManager::Server_ForceCallout_Implementation(const FName& CalloutType)
{
	ForceCallout(CalloutType);
}

FCallouts ACalloutsManager::GetCalloutData(const FName& CalloutType) const
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
