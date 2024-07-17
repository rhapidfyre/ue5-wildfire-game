// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/CalloutsManager.h"

#include "Logging/StructuredLog.h"


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

		GetWorldTimerManager().SetTimer(CalloutTimerHandle, this, &ACalloutsManager::GenerateCallout, 300.0f, true);
	}
	else
	{
		Destroy();
	}
}

ACalloutsManager* ACalloutsManager::GetInstance(UWorld* World)
{
	if (Instance == nullptr)
	{
		FActorSpawnParameters SpawnParams;
		Instance = World->SpawnActor<ACalloutsManager>(SpawnParams);
	}

	return Instance;
}

void ACalloutsManager::GenerateCallout()
{
	UE_LOGFMT(LogTemp, Warning, "ACalloutsManager({NetMode}): GenerateCallout()"
		, HasAuthority() ? "SRV" : "CLI");
}
