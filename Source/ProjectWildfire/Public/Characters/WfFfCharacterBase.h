// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfCharacterBase.h"
#include "Delegates/Delegate.h"
#include "Gas/WfAbilityComponent.h"
#include "Interfaces/WfFireStationInterface.h"

#include "WfFfCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHourlyRateChanged,
	const float&, OldHourlyRate, const float&, NewHourlyRate);

/**
 * \brief Base class for firefighter characters
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API AWfFfCharacterBase : public AWfCharacterBase, public IWfFireStationInterface
{
	GENERATED_BODY()

public:

	AWfFfCharacterBase();

	UFUNCTION(BlueprintCallable)
	void SetHourlyRate(float NewHourlyRate = 3.0f);

	UFUNCTION(BlueprintPure)
	float GetHourlyRate() const { return HourlyRate; }

	// Implement the IOverlapDetector interface
	virtual void EventBeginOverlap(AActor* OverlappedActor) override;

	virtual void EventEndOverlap(AActor* OverlappedActor) override;

	virtual bool IsEligible() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void LoadCharacter(USaveGame* CharacterSaveGame) override;

	virtual void NewCharacter(const FGameplayTag& NewPrimaryRole) override;

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Actor Component")
	UWfAbilityComponent* AbilityComponent;

	UPROPERTY(BlueprintAssignable)
	FOnHourlyRateChanged OnHourlyRateChanged;

private:
	bool bInQuarters;

	int YearsOfService;

	int YearsInGrade;

	float HourlyRate;

};
