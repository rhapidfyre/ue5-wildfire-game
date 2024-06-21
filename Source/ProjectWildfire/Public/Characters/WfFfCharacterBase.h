// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfCharacterBase.h"
#include "Interfaces/WfFireStationInterface.h"

#include "WfFfCharacterBase.generated.h"

/**
 * \brief Base class for firefighter characters
 */
UCLASS()
class PROJECTWILDFIRE_API AWfFfCharacterBase : public AWfCharacterBase, public IWfFireStationInterface
{
	GENERATED_BODY()

public:

	AWfFfCharacterBase();

	// Implement the IOverlapDetector interface
	virtual void EventBeginOverlap(AActor* OverlappedActor) override;
	virtual void EventEndOverlap(AActor* OverlappedActor) override;
	virtual bool IsEligible() override;

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	bool bInQuarters;

};
