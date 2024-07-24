// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfCharacterBase.h"
#include "Delegates/Delegate.h"
#include "Gas/WfAbilityComponent.h"
#include "Interfaces/WfFireStationInterface.h"
#include "Saves/WfCharacterSaveGame.h"

#include "WfFfCharacterBase.generated.h"

class AWfFireApparatusBase;
class AWfFireStationBase;
class UWfScheduleComponent;

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

	virtual void SaveCharacter() override;

	UFUNCTION(BlueprintCallable)
	void SetHourlyRate(float NewHourlyRate = 3.0f);

	UFUNCTION(BlueprintPure)
	FJobContractData GetFirefighterJobContract();

	UFUNCTION(BlueprintPure)
	float GetHourlyRate() const { return HourlyRate; }

	void SetJobContract(const FString& NewContractId, const int32 ContractIndex);

	// Implement the IOverlapDetector interface
	virtual void EventBeginOverlap(AActor* OverlappedActor) override;

	virtual void EventEndOverlap(AActor* OverlappedActor) override;

	virtual bool IsEligible() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void LoadCharacter(USaveGame* CharacterSaveGame) override;

	virtual void NewCharacter(const FGameplayTag& NewPrimaryRole) override;

	UFUNCTION(BlueprintPure)
	bool IsInQuarters() const { return bInQuarters; }

	UFUNCTION(BlueprintPure)
	bool CanDriveApparatus() const;


protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintAssignable)
	FOnHourlyRateChanged OnHourlyRateChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Actor Component")
	UWfScheduleComponent* ScheduleComponent;

private:

	UPROPERTY(Replicated) bool bInQuarters;

	UPROPERTY(Replicated) int YearsOfService;

	UPROPERTY(Replicated) int YearsInGrade;

	UPROPERTY(Replicated) float HourlyRate;

	FString ContractId;

	int32 ContractUserIndex;

};
