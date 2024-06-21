// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/WfFireApparatusBase.h"


AWfFireApparatusBase::AWfFireApparatusBase()
{
	PrimaryActorTick.bCanEverTick = true;
	IdentityStation   = 1;
	IdentityApparatus = "Utility";
	IdentityUnique    = 1;
}

FString AWfFireApparatusBase::GetApparatusIdentity() const
{
	if (!IdentityOverride.IsEmpty())
	{
		return IdentityOverride;
	}

	const FString StringSign = FString::Printf(
		TEXT("%s %d-%02d"), *IdentityApparatus, IdentityStation, IdentityUnique);

	return FString(IdentityApparatus + " " + FString::FromInt(IdentityStation) + StringSign);
}

void AWfFireApparatusBase::SetApparatusIdentity(FString NewIdentity)
{
	const FString OldIdentity = GetApparatusIdentity();
	IdentityOverride = NewIdentity;
	if (OnApparatusIdentityChanged.IsBound())
	{
		OnApparatusIdentityChanged.Broadcast( OldIdentity, GetApparatusIdentity() );
	}
}

void AWfFireApparatusBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfFireApparatusBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWfFireApparatusBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
