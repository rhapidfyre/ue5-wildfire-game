// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/WfFireApparatusBase.h"

#include "Net/UnrealNetwork.h"


AWfFireApparatusBase::AWfFireApparatusBase()
{
	PrimaryActorTick.bCanEverTick = true;
	IdentityStation   = 1;
	IdentityType = "Utility";
	IdentityUnique    = 1;
}

void AWfFireApparatusBase::SetIdentities(const int32 StationNumber, const FString& ApparatusType,
	const int32 UniqueNumber)
{
	IdentityStation = StationNumber;
	IdentityType = ApparatusType;
	IdentityUnique = UniqueNumber;
}

FString AWfFireApparatusBase::GetApparatusIdentity() const
{
	if (!IdentityOverride.IsEmpty())
	{
		return IdentityOverride;
	}

	const FString StringSign = FString::Printf(
		TEXT("%s %d%02d"), *IdentityType, IdentityStation, IdentityUnique);

	return StringSign;
}

void AWfFireApparatusBase::SetApparatusIdentity(FString NewIdentity)
{
	const FString OldIdentity = GetApparatusIdentity();
	IdentityOverride = NewIdentity;
}

void AWfFireApparatusBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfFireApparatusBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWfFireApparatusBase, IdentityOverride);
	DOREPLIFETIME(AWfFireApparatusBase, IdentityStation);
	DOREPLIFETIME(AWfFireApparatusBase, IdentityType);
	DOREPLIFETIME(AWfFireApparatusBase, IdentityUnique);
}

void AWfFireApparatusBase::OnRep_IdentityUnique_Implementation(const int32 OldIdentityValue)
{
	const FString OldCallsign = FString::Printf(
		TEXT("%s %d%02d"), *IdentityType, IdentityStation, OldIdentityValue);
	if (OnApparatusIdentityChanged.IsBound())
		OnApparatusIdentityChanged.Broadcast(OldCallsign, GetApparatusIdentity());
}

void AWfFireApparatusBase::OnRep_IdentityApparatus_Implementation(const FString& OldIdentityValue)
{
	const FString OldCallsign = FString::Printf(
		TEXT("%s %d%02d"), *OldIdentityValue, IdentityStation, IdentityUnique);
	if (OnApparatusIdentityChanged.IsBound())
		OnApparatusIdentityChanged.Broadcast(OldCallsign, GetApparatusIdentity());
}

void AWfFireApparatusBase::OnRep_IdentityStation_Implementation(const int32 OldIdentityValue)
{
	const FString OldCallsign = FString::Printf(
		TEXT("%s %d%02d"), *IdentityType, OldIdentityValue, IdentityUnique);
	if (OnApparatusIdentityChanged.IsBound())
		OnApparatusIdentityChanged.Broadcast(OldCallsign, GetApparatusIdentity());
}

void AWfFireApparatusBase::OnRep_IdentityOverride_Implementation(const FString& OldIdentityValue)
{
	if (OnApparatusIdentityChanged.IsBound())
		OnApparatusIdentityChanged.Broadcast(OldIdentityValue, GetApparatusIdentity());
}

void AWfFireApparatusBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWfFireApparatusBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
