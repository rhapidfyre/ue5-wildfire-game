// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerController.h"

#include "WfPlayerControllerBase.generated.h"

// Delegates and forward declares for delegates
struct FInputActionValue;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHotkeyTriggered, const UInputAction*, HotkeyAction, const FInputActionValue&, InputValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnActorSelected, AActor*, OldSelection, AActor*, NewSelection);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnActorHitBySelection, const FVector&, ImpactPosition, const bool, bPrimary, AActor*, HitActor);

// Forward declarations for implementation
class UInputMappingContext;

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FWfInputActionMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FName FunctionName;

	FWfInputActionMap()
		: InputAction(nullptr), FunctionName(NAME_None)
	{}

	FWfInputActionMap(UInputAction* InInputAction, FName InFunctionName)
		: InputAction(InInputAction), FunctionName(InFunctionName)
	{}
};


/**
 * \brief The base player controller class for all player controllers
 *    Currently, this is the only controller players need, so if
 *    future logic needs inheriting, some methods/members will
 *    need to be relocated to child player controllers.
 */
UCLASS(BlueprintType)
class PROJECTWILDFIRE_API AWfPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:

	AWfPlayerControllerBase(); // Default Constructor

	UFUNCTION(BlueprintPure) AActor* GetSelectedActor() const { return SelectedActor; }

	virtual void SetSelectedActor(AActor* NewSelection);

	UFUNCTION() void ClearSelectedActor();

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void SetupInputComponent() override;

	// Custom virtual that allows setting up the enhanced input contexts
	virtual void SetupEnhancedInput();

	// Custom virtual that allows binding input actions to methods
	virtual void SetupInputActions();

private:

	void SetAlwaysMouseControl(bool bSetValue = true);

	bool HandleClick(bool bIsPrimary = true);

	UFUNCTION() void HandleClickPrimary();
	UFUNCTION() void HandleClickSecondary();

	UFUNCTION() void MoveCamera(const FInputActionValue& Value);
	UFUNCTION() void RotateCamera(const FInputActionValue& Value);
	UFUNCTION() void ZoomCamera(const FInputActionValue& Value);
	UFUNCTION() void LockCameraPitch(const FInputActionValue& Value);
	UFUNCTION() void LockCameraMove(const FInputActionValue& Value);

	UFUNCTION() void MouseCameraRotate();
	UFUNCTION() void MouseCameraMove();

	void SetSavedMousePosition();

public:

	UPROPERTY(BlueprintAssignable)	FOnHotkeyTriggered	OnHotkeyTriggered;
	UPROPERTY(BlueprintAssignable)	FOnActorSelected	OnActorSelected;
	UPROPERTY(BlueprintAssignable)  FOnActorHitBySelection OnActorHitBySelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TMap<UInputMappingContext*, int32> DefaultMappings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<FWfInputActionMap> InputActionBindings;

	// When this input is held down, the movement of the mouse will rotate the camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MousePitchLock;

	// When this input is held down, the movement of the mouse will move the camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MouseMoveLock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertMouseLeftRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bInvertMouseUpDown = true;

private:

	FTimerHandle MouseTimer;
	FVector2D MouseScreenPosition;
	FVector2D MouseLastPosition;

	float MouseSensitivityMove  = 5.0f;
	float MouseSensitivityPitch = 0.2f;

	UPROPERTY()	AActor* SelectedActor;

	bool bAllowCamMoveFreely;
};
