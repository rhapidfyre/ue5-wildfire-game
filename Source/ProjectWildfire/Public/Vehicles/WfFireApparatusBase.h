﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfVehicleBase.h"
#include "Delegates/Delegate.h"

#include "WfFireApparatusBase.generated.h"

class AWfFfCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnApparatusIdentityChanged, const FString&, OldIdentity, const FString&, NewIdentity);

/**
 * \brief The base class for all fire vehicles, including ambulances.
 * This class defines all common methods and members that will be used by
 * all fire department vehicles, such as engines, ladders, ambulances, and squads.
 */
UCLASS(BlueprintType)
class PROJECTWILDFIRE_API AWfFireApparatusBase : public AWfVehicleBase
{
	GENERATED_BODY()

public:

	AWfFireApparatusBase();

	UFUNCTION(BlueprintCallable)
	void SetIdentities(
		const int32 StationNumber, const FString& ApparatusType, const int32 UniqueNumber);

	UFUNCTION(BlueprintPure)
	TArray<AWfFfCharacterBase*> GetAssignedFirefighters() const { return AssignedFirefighters; }

	UFUNCTION(BlueprintCallable)
	void SetFirefighterAssigned(AWfFfCharacterBase* FireFighter, const bool bAssign = true);

	UFUNCTION(BlueprintPure) FString GetApparatusIdentity() const;
	UFUNCTION(BlueprintPure) int32 GetApparatusIdentityStation() const { return IdentityStation; }
	UFUNCTION(BlueprintPure) FString GetApparatusIdentityType() const { return IdentityType; }
	UFUNCTION(BlueprintPure) int32 GetApparatusIdentityUnique() const { return IdentityUnique; }

	UFUNCTION(BlueprintCallable) void
	SetApparatusIdentity(FString NewIdentity);

	void SetFireStationIdentity(const int32 NewIdentity) { IdentityStation = NewIdentity; }
	void SetUniqueIdentity(const int32 NewIdentity) { IdentityStation = NewIdentity; }

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_SetFirefighterAssigned(AWfFfCharacterBase* FireFighter, const bool bAssign);

public: // Public Members

	UPROPERTY(BlueprintAssignable) FOnApparatusIdentityChanged OnApparatusIdentityChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	FString FireApparatusType = "None";

private: // Private Members

	UFUNCTION(NetMulticast, Reliable) void OnRep_IdentityOverride(const FString& OldIdentityValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_IdentityStation(const int32 OldIdentityValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_IdentityApparatus(const FString& OldIdentityValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_IdentityUnique(const int32 OldIdentityValue);

	// If valid, overrides the generated call signs
	UPROPERTY(ReplicatedUsing=OnRep_IdentityOverride)   FString IdentityOverride;

	UPROPERTY(ReplicatedUsing=OnRep_IdentityStation)   int32   IdentityStation;
	UPROPERTY(ReplicatedUsing=OnRep_IdentityApparatus) FString IdentityType;
	UPROPERTY(ReplicatedUsing=OnRep_IdentityUnique)    int32   IdentityUnique;

	UPROPERTY(Replicated) TArray<AWfFfCharacterBase*> AssignedFirefighters;
};
