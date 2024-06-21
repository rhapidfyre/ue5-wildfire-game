// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/WfVehicleBase.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"


AWfVehicleBase::AWfVehicleBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create a spring arm component
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 250.0f; // The camera follows at this distance behind the character
    SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a camera and attach to spring arm
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false; // Don't rotate camera with arm

    // Setup vehicle movement
    UChaosWheeledVehicleMovementComponent* VehicleMovement =
        CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    VehicleMovement->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
    VehicleMovement->TransmissionSetup.bUseAutomaticGears = true;

    FVehicleSteeringConfig SteeringConfig = VehicleMovement->SteeringSetup;
    SteeringConfig.SteeringCurve.GetRichCurve()->AddKey(0.f, 1.f);
    SteeringConfig.SteeringCurve.GetRichCurve()->AddKey(40.f, 0.7f);
    SteeringConfig.SteeringCurve.GetRichCurve()->AddKey(120.f, 0.6f);
}

void AWfVehicleBase::BeginPlay()
{
    Super::BeginPlay();
}

void AWfVehicleBase::SetupMappingContexts()
{
    const APlayerController* MyController = Cast<APlayerController>(GetController());
    if (IsValid(MyController))
    {
        UEnhancedInputLocalPlayerSubsystem* MyInputSubsystem = ULocalPlayer::GetSubsystem
            <UEnhancedInputLocalPlayerSubsystem>(MyController->GetLocalPlayer());
        if (IsValid(MyInputSubsystem))
        {
            MyInputSubsystem->AddMappingContext(VehicleInputMappingContext, 1);
        }
    }
}

void AWfVehicleBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWfVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    if (MoveForwardAction)
    {
        EnhancedInputComponent->BindAction(
            MoveForwardAction, ETriggerEvent::Triggered, this, &AWfVehicleBase::MoveForward);
    }

    if (MoveRightAction)
    {
        EnhancedInputComponent->BindAction(
            MoveRightAction, ETriggerEvent::Triggered, this, &AWfVehicleBase::MoveRight);
    }

    if (HandbrakeAction)
    {
        EnhancedInputComponent->BindAction(
            HandbrakeAction, ETriggerEvent::Started, this, &AWfVehicleBase::OnHandbrakePressed);

        EnhancedInputComponent->BindAction(
            HandbrakeAction, ETriggerEvent::Completed, this, &AWfVehicleBase::OnHandbrakeReleased);
    }
}

void AWfVehicleBase::MoveForward(const FInputActionValue& Value)
{
    float ForwardValue = Value.Get<float>();
    GetVehicleMovementComponent()->SetThrottleInput(ForwardValue);
}

void AWfVehicleBase::MoveRight(const FInputActionValue& Value)
{
    float RightValue = Value.Get<float>();
    GetVehicleMovementComponent()->SetSteeringInput(RightValue);
}

void AWfVehicleBase::OnHandbrakePressed()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AWfVehicleBase::OnHandbrakeReleased()
{
    GetVehicleMovementComponent()->SetHandbrakeInput(false);
}
