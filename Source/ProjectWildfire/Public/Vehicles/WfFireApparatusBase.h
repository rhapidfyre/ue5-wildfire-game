// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfVehicleBase.h"
#include "Delegates/Delegate.h"

#include "WfFireApparatusBase.generated.h"

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

	UFUNCTION(BlueprintPure) FString GetApparatusIdentity() const;
	UFUNCTION(BlueprintCallable) void SetApparatusIdentity(FString NewIdentity);

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;

public: // Public Members

	UPROPERTY(BlueprintAssignable) FOnApparatusIdentityChanged OnApparatusIdentityChanged;

private: // Private Members

	// If valid, overrides the generated call signs
	FString IdentityOverride;

	int32   IdentityStation;
	FString IdentityApparatus;
	int32   IdentityUnique;
};
