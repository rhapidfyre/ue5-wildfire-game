#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "WfPlayerPawnBase.generated.h"

class UFloatingPawnMovement;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;


UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfPlayerPawnBase : public APawn
{
	GENERATED_BODY()

public:
	AWfPlayerPawnBase();

	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	void CamMovement(const FInputActionValue& Value);
	void CamRotation(const FInputActionValue& Value);
	void CamDistance(const FInputActionValue& Value);

	void CamMoveDelta(const FVector2D& MoveDelta);
	void CamPitch(const FVector2D& PitchDelta);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* FloatingMovementComponent;

	float CamMoveSpeed      =  1000.0f;
	float CamRotateSpeed    =   100.0f;
	float CamZoomMultiplier = 	  1.0f;
	float CamDistanceMin    =   200.0f;
	float CamDistanceMax    = 12000.0f;

};
