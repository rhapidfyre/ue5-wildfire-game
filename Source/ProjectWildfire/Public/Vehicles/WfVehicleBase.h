// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"

#include "WfVehicleBase.generated.h"


class UInputComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * \brief The base class for all land-based vehicles, such as sedans, trucks,
 * motorcycles. Anything that can be driven with wheels used for transportation.
 * This is the base class for ALL vehicles, civilian or emergency.
 * For fire department vehicles, see child class 'AWfFireApparatusBase'
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfVehicleBase : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:

	AWfVehicleBase();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Actor Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Actor Components")
    UCameraComponent* Camera;

protected:

	virtual void BeginPlay() override;

	virtual void SetupMappingContexts();

private:

	void MoveForward(const FInputActionValue& Value);

	void MoveRight(const FInputActionValue& Value);

	void OnHandbrakePressed();

	void OnHandbrakeReleased();

	// Enhanced Input System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* VehicleInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* HandbrakeAction;

};
