// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WfFfCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "Logging/StructuredLog.h"


AWfFfCharacterBase::AWfFfCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bInQuarters = false;
}

void AWfFfCharacterBase::EventBeginOverlap(AActor* OverlappedActor)
{
	//IWfFireStationInterface::EventBeginOverlap(OverlappedActor);
	bInQuarters = true;
	UE_LOGFMT(LogTemp, Display, "{CharacterName} has returned to quarters.", GetName());
}

void AWfFfCharacterBase::EventEndOverlap(AActor* OverlappedActor)
{
	//IWfFireStationInterface::EventEndOverlap(OverlappedActor);
	bInQuarters = false;
	UE_LOGFMT(LogTemp, Display, "{CharacterName} has left the station.", GetName());
}

bool AWfFfCharacterBase::IsEligible()
{
	return IWfFireStationInterface::IsEligible();
}

void AWfFfCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfFfCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_EngineTraceChannel2, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}

void AWfFfCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWfFfCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
