// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Characters/WfPlayerPawnBase.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Controllers/WfPlayerControllerBase.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Logging/StructuredLog.h"


AWfPlayerPawnBase::AWfPlayerPawnBase()
{
    // Don't rotate character to camera direction
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create a camera boom...
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = 2000.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;

    // Create a camera...
    TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false;

    // Create a floating movement component
    FloatingMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovementComponent"));

    // Activate ticking in order to update the cursor every frame.
    PrimaryActorTick.bCanEverTick = true;

    PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWfPlayerPawnBase::BeginPlay()
{
    Super::BeginPlay();
}

void AWfPlayerPawnBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWfPlayerPawnBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    AWfPlayerControllerBase* PlayerController = Cast<AWfPlayerControllerBase>(NewController);

}

void AWfPlayerPawnBase::UnPossessed()
{
    Super::UnPossessed();
}

void AWfPlayerPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* EiComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (IsValid(EiComponent))
    {

    }
}

void AWfPlayerPawnBase::CamMovement(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    AWfPlayerControllerBase* PlayerController = Cast<AWfPlayerControllerBase>( GetController() );
    if (!IsValid(PlayerController))
    {
        return;
    }

    // Get the forward and right vector based on the camera's rotation
    const FRotator CameraRotation = CameraBoom->GetComponentRotation();
    const FRotator YawRotation(0, CameraRotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Add movement input
    FloatingMovementComponent->AddInputVector(ForwardDirection
        * MovementVector.Y * CamMoveSpeed * GetWorld()->GetDeltaSeconds());

    FloatingMovementComponent->AddInputVector(RightDirection
        * MovementVector.X * CamMoveSpeed * GetWorld()->GetDeltaSeconds());
}

void AWfPlayerPawnBase::CamRotation(const FInputActionValue& Value)
{
    FVector2D RotationVector = Value.Get<FVector2D>();

    // Adjust the camera boom's rotation based on input
    FRotator NewRotation = CameraBoom->GetRelativeRotation();
    NewRotation.Yaw += RotationVector.X * CamRotateSpeed * GetWorld()->GetDeltaSeconds();
    NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + RotationVector.Y * CamRotateSpeed * GetWorld()->GetDeltaSeconds(), -80.0f, 80.0f);

    CameraBoom->SetRelativeRotation(NewRotation);
}

void AWfPlayerPawnBase::CamDistance(const FInputActionValue& Value)
{
    float ZoomValue = Value.Get<float>();

    // Adjust the camera boom's length based on input
    float NewTargetArmLength = CameraBoom->TargetArmLength - (ZoomValue * 100.f * CamZoomMultiplier);
    CameraBoom->TargetArmLength = FMath::Clamp(NewTargetArmLength, CamDistanceMin, CamDistanceMax);

}

void AWfPlayerPawnBase::CamMoveDelta(const FVector2D& MoveDelta)
{
    float SensitivityX = 1.0f;
    float SensitivityY = 1.0f;
    float MoveRightDelta   = MoveDelta.X * SensitivityX;
    float MoveForwardDelta = MoveDelta.Y * SensitivityY;

    // Get the forward and right vector based on the camera's rotation
    const FRotator CameraRotation = CameraBoom->GetComponentRotation();
    const FRotator YawRotation(0, CameraRotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Add movement input
    FloatingMovementComponent->AddInputVector(ForwardDirection
        * MoveForwardDelta * CamMoveSpeed * GetWorld()->GetDeltaSeconds());

    FloatingMovementComponent->AddInputVector(RightDirection
        * MoveRightDelta * CamMoveSpeed * GetWorld()->GetDeltaSeconds());

}

void AWfPlayerPawnBase::CamPitch(const FVector2D& PitchDelta)
{
    float SensitivityX = 1.0f;
    float SensitivityY = 1.0f;
    float RotateValue = PitchDelta.X * SensitivityX;
    float PitchValue  = PitchDelta.Y * SensitivityY;

    FRotator OldRotation = CameraBoom->GetRelativeRotation();
    FRotator NewRotation = OldRotation;

    // Clamp between -90 to 90 to avoid going upside-down
    NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + PitchValue, -89.0f, 89.0f);
    NewRotation.Yaw = FMath::Clamp(NewRotation.Yaw + RotateValue, -89.0f, 89.0f);

    CameraBoom->SetRelativeRotation(NewRotation);

    UE_LOGFMT(LogTemp, Display, "{PawnBase}({NetMode}): Camera Pitch Changed ({OldPitch}) ({NewPitch})",
        GetName(), HasAuthority()?"SRV":"CLI", OldRotation.Pitch, NewRotation.Pitch);
}
