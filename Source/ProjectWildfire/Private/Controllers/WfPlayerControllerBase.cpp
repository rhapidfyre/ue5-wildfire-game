// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WfPlayerControllerBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Characters/WfPlayerPawnBase.h"
#include "Interfaces/WfClickableInterface.h"
#include "Logging/StructuredLog.h"


AWfPlayerControllerBase::AWfPlayerControllerBase()
	: MousePitchLock(nullptr)
		, SelectedActor(nullptr)
{
	bAllowCamMoveFreely = true;
}

void AWfPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	SetupInputActions();		// Single Input Actions
	SetupEnhancedInput();		// Mapping Contexts
    SetAlwaysMouseControl();
}

void AWfPlayerControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

/**
 * \brief Sets up static input bindings that will never change or be rebound
 * \obsolete Most likely obsolete
 */
void AWfPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EiComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EiComponent))
	{

	}
}

/**
 * \brief Sets up the input mapping contexts and bind actions for the Enhanced Input System
 * \n Called after parent's BeginPlay returns
 */
void AWfPlayerControllerBase::SetupEnhancedInput()
{
	UEnhancedInputComponent* EiComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EiComponent))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
		if (IsValid(Subsystem))
		{
			for (const TPair<UInputMappingContext*, int32>& DefaultMapping : DefaultMappings)
			{
				// By default, use the default and idle mapping contexts
				if (IsValid(DefaultMapping.Key))
					Subsystem->AddMappingContext(DefaultMapping.Key, DefaultMapping.Value);
			}
		}
	}
}

void AWfPlayerControllerBase::SetSelectedActor(AActor* NewSelection)
{
	AActor* OldSelection = SelectedActor;
	SelectedActor = NewSelection;
	if (IsValid(SelectedActor))
	{
		UE_LOGFMT(LogTemp, Display,
			"PlayerCtrl({NetMode}): '{HitActor}' Selected",
			HasAuthority()?"SRV":"CLI", NewSelection->GetName());
	}
	else
	{
		UE_LOGFMT(LogTemp, Display,
			"PlayerCtrl({NetMode}): Actor Selection Cleared", HasAuthority()?"SRV":"CLI");
	}
	if (OnActorSelected.IsBound())
		OnActorSelected.Broadcast(OldSelection, SelectedActor);
}

void AWfPlayerControllerBase::ClearSelectedActor()
{
	SetSelectedActor(nullptr);
}

/**
 * \brief Sets up the input bindings, triggering delegates when the input is triggered
 */
void AWfPlayerControllerBase::SetupInputActions()
{
	UEnhancedInputComponent* EiComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EiComponent))
	{
		// Dynamic Bindings
		for (const FWfInputActionMap& Mapping : InputActionBindings)
		{
			if (IsValid(Mapping.InputAction) && !Mapping.FunctionName.IsNone())
			{
				EiComponent->BindAction(
					Mapping.InputAction, ETriggerEvent::Triggered, this, Mapping.FunctionName);
			}
		}

		// Static Bindings
		if (IsValid(MousePitchLock))
		{
			EiComponent->BindAction(
				MousePitchLock, ETriggerEvent::Started, this, &AWfPlayerControllerBase::LockCameraPitch);
			EiComponent->BindAction(
				MousePitchLock, ETriggerEvent::Completed, this, &AWfPlayerControllerBase::LockCameraPitch);
		}

		if (IsValid(MouseMoveLock))
		{
			EiComponent->BindAction(
				MouseMoveLock, ETriggerEvent::Started, this, &AWfPlayerControllerBase::LockCameraMove);
			EiComponent->BindAction(
				MouseMoveLock, ETriggerEvent::Completed, this, &AWfPlayerControllerBase::LockCameraMove);
		}
	}
}

void AWfPlayerControllerBase::SetAlwaysMouseControl(bool bSetValue)
{
	// Set the input mode to Game and UI, allowing for mouse input at all times
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(!bSetValue);
	SetInputMode(InputMode);

	// Ensure the mouse cursor is always visible
	bShowMouseCursor = bSetValue;
}

bool AWfPlayerControllerBase::HandleClick(bool bIsPrimary)
{
	// Find impact point to ground
	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_WorldStatic, false, HitResult))
	{
		UE_LOGFMT(LogTemp, Display, "PlayerCtrl({NetMode}): HandleClick({PriSec}) found no world impact location.",
			HasAuthority()?"SRV":"CLI", bIsPrimary?"PRI":"SEC");
		return false;
	}

	FVector ImpactPoint = HitResult.ImpactPoint;

	// Perform Sphere Trace at impact point
	TArray<FHitResult> SphereHits;
	if (GetWorld()->SweepMultiByChannel(
		SphereHits, ImpactPoint, ImpactPoint,
		FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(32.0f),
		FCollisionQueryParams(SCENE_QUERY_STAT(HandleClick), false, GetPawn())))
	{
		for (const FHitResult& ThisResult : SphereHits)
		{
			AActor* HitActor = ThisResult.GetActor();
			if (IsValid(HitActor))
			{
				// Did the hit event hit something that is clickable?
				if (HitActor->GetClass()->ImplementsInterface(UWfClickableInterface::StaticClass()))
				{
					IWfClickableInterface* Clickable = Cast<IWfClickableInterface>(HitActor);
					if (bIsPrimary)
					{
						if (Clickable->EventPrimarySelect())
						{
							SetSelectedActor(HitActor);
						}
					}
					else
					{
						if (Clickable->EventSecondarySelect())
						{
							SetSelectedActor(HitActor);
						}
					}
					return true; // Successful select event dispatched
				}
				// The trace hit a Pawn but it wasn't a clickable pawn (ground, building, gate, door, etc.)
				//     Broadcast the event and let the listeners handle it
				if (OnActorHitBySelection.IsBound())
					OnActorHitBySelection.Broadcast(ThisResult.ImpactPoint, bIsPrimary, HitActor);
			}
		}

	}
	return false;
}

void AWfPlayerControllerBase::HandleClickPrimary()
{
	// Disables primary selection if the camera is moving
	if (!GetWorldTimerManager().IsTimerActive(MouseTimer)) return;
	const bool bClickHandled = HandleClick();
	UE_LOGFMT(LogTemp, Display, "HandleClickPrimary(): {HandledResponse}", bClickHandled ? "Handled" : "Unhandled");
}

void AWfPlayerControllerBase::HandleClickSecondary()
{
	// Disables secondary selection if the camera is moving
	if (!GetWorldTimerManager().IsTimerActive(MouseTimer)) return;
	const bool bClickHandled = HandleClick(false);
	UE_LOGFMT(LogTemp, Display, "HandleClickSecondary(): {HandledResponse}", bClickHandled ? "Handled" : "Unhandled");
}

void AWfPlayerControllerBase::MoveCamera(const FInputActionValue& Value)
{
	if (!bAllowCamMoveFreely) return;
	// Pass controls to generic player pawn
	AWfPlayerPawnBase* PawnBase = Cast<AWfPlayerPawnBase>(GetPawn());
	if (IsValid(PawnBase))
		PawnBase->CamMovement(Value);
}

void AWfPlayerControllerBase::RotateCamera(const FInputActionValue& Value)
{
	if (!bAllowCamMoveFreely) return;
	// Pass controls to generic player pawn
	AWfPlayerPawnBase* PawnBase = Cast<AWfPlayerPawnBase>(GetPawn());
	if (IsValid(PawnBase))
		PawnBase->CamRotation(Value);
}

void AWfPlayerControllerBase::ZoomCamera(const FInputActionValue& Value)
{
	if (!bAllowCamMoveFreely) return;
	// Pass controls to generic player pawn
	AWfPlayerPawnBase* PawnBase = Cast<AWfPlayerPawnBase>(GetPawn());
	if (IsValid(PawnBase))
		PawnBase->CamDistance(Value);
}

void AWfPlayerControllerBase::LockCameraPitch(const FInputActionValue& Value)
{
	bool bUnlockCam   = Value.IsNonZero();
	bool bIsCamMoving = GetWorldTimerManager().IsTimerActive(MouseTimer);
	if (bUnlockCam && bIsCamMoving)
	{
		UE_LOGFMT(LogTemp, Warning, "Cannot Authorize Mouse Movement - Already Moving!");
		return;
	}

	if (bIsCamMoving && !bUnlockCam)
	{
		GetWorldTimerManager().ClearTimer(MouseTimer);
		SetAlwaysMouseControl(true);
		UE_LOGFMT(LogTemp, Display, "Mouse Rotation Disabled");
	}
	else
	{
		SetSavedMousePosition();
		FTimerDelegate MouseDelegate;
		MouseDelegate.BindUObject(this, &AWfPlayerControllerBase::MouseCameraRotate);
		GetWorldTimerManager().SetTimer(MouseTimer, MouseDelegate, 0.01f, true);
		UE_LOGFMT(LogTemp, Display, "Mouse Rotation Enabled");
	}
}

void AWfPlayerControllerBase::LockCameraMove(const FInputActionValue& Value)
{
	bool bUnlockCam   = Value.IsNonZero();
	bool bIsCamMoving = GetWorldTimerManager().IsTimerActive(MouseTimer);
	if (bUnlockCam && bIsCamMoving)
	{
		UE_LOGFMT(LogTemp, Warning, "Cannot Authorize Mouse Movement - Already Moving!");
		return;
	}

	if (bIsCamMoving && !bUnlockCam)
	{
		GetWorldTimerManager().ClearTimer(MouseTimer);
		SetAlwaysMouseControl(true);
		UE_LOGFMT(LogTemp, Display, "Mouse Movement Disabled");
	}
	else
	{
		SetSavedMousePosition();
		FTimerDelegate MouseDelegate;
		MouseDelegate.BindUObject(this, &AWfPlayerControllerBase::MouseCameraMove);
		GetWorldTimerManager().SetTimer(MouseTimer, MouseDelegate, 0.01f, true);
		UE_LOGFMT(LogTemp, Display, "Mouse Movement Enabled");
	}
}

/**
 * \brief Finds the difference between the mouse movement from the last known position
 * to change the camera pitch.
 */
void AWfPlayerControllerBase::MouseCameraRotate()
{
	if (!bAllowCamMoveFreely)
	{
		GetWorldTimerManager().ClearTimer(MouseTimer);
		UE_LOGFMT(LogTemp, Warning, "Camera Rotation has been Disabled - Mouse Controls Released");
		return;
	}

	float MouseX, MouseY;
	GetMousePosition(MouseX, MouseY);

	FVector2D NewMousePosition = FVector2D(MouseX, MouseY);
	FVector2D MousePositionDelta = NewMousePosition - MouseLastPosition;
	MouseLastPosition = NewMousePosition;

	if (!MousePositionDelta.IsNearlyZero(0.1))
	{
		if (bInvertMouseLeftRight)
			MousePositionDelta.X *= -1;

		if (bInvertMouseUpDown)
			MousePositionDelta.Y *= -1;

		AWfPlayerPawnBase* PawnBase = Cast<AWfPlayerPawnBase>(GetPawn());
		if (IsValid(PawnBase))
		{
			PawnBase->CamPitch(MousePositionDelta * MouseSensitivityMove);
		}
	}
}

/**
 * \brief Finds the difference between the mouse movement from the last known position
 * to change the camera's world location.
 */
void AWfPlayerControllerBase::MouseCameraMove()
{
	if (!bAllowCamMoveFreely)
	{
		GetWorldTimerManager().ClearTimer(MouseTimer);
		UE_LOGFMT(LogTemp, Warning, "Camera Movement has been Disabled - Mouse Controls Released");
		return;
	}

	if (!bAllowCamMoveFreely) return;

	float MouseX, MouseY;
	GetMousePosition(MouseX, MouseY);

	FVector2D NewMousePosition = FVector2D(MouseX, MouseY);
	FVector2D MousePositionDelta = NewMousePosition - MouseLastPosition;
	//MouseLastPosition = NewMousePosition;

	if (!MousePositionDelta.IsNearlyZero(0.1))
	{
		if (bInvertMouseLeftRight)
			MousePositionDelta.X *= -1;

		if (bInvertMouseUpDown)
			MousePositionDelta.Y *= -1;

		AWfPlayerPawnBase* PawnBase = Cast<AWfPlayerPawnBase>(GetPawn());
		if (IsValid(PawnBase))
		{
			PawnBase->CamMoveDelta(MousePositionDelta * MouseSensitivityMove);
		}
	}
}

void AWfPlayerControllerBase::SetSavedMousePosition()
{
	float MouseX, MouseY;
	GetMousePosition(MouseX, MouseY);
	MouseScreenPosition = FVector2D(MouseX, MouseY);
	MouseLastPosition   = MouseScreenPosition;
}
