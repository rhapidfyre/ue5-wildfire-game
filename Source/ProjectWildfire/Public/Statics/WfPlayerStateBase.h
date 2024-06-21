// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerState.h"

#include "WfPlayerStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourceUpdated, const FGameplayTag&, ResourceTag, const float, OldValue, const float, NewValue);


/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:

	AWfPlayerStateBase();

	UFUNCTION(BlueprintPure)
	float GetResourceValue(const FGameplayTag& ResourceTag) const;

	UFUNCTION(BlueprintPure)
	FGameplayTagContainer GetAllResourceTags() const;

	UFUNCTION(BlueprintCallable)	float AddMoney(float AddValue = 1.0f);
	UFUNCTION(BlueprintCallable)	float AddKilowattUsage(float AddValue = 1.0f);
	UFUNCTION(BlueprintCallable)	float AddOxygenReserve(float AddValue = 1.0f);
	UFUNCTION(BlueprintCallable)	float AddWaterStorage(float AddValue = 1.0f);

	UFUNCTION(BlueprintCallable)
	float RemoveMoney(const float RemoveValue = 1.0f) { return AddMoney(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveKilowattUsage(const float RemoveValue = 1.0f) { return AddKilowattUsage(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveOxygenReserve(const float RemoveValue = 1.0f) { return AddOxygenReserve(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveWaterStorage(const float RemoveValue = 1.0f) { return AddWaterStorage(-1 * RemoveValue); }

	UFUNCTION(BlueprintPure)
	float GetMoney() const;

	UFUNCTION(BlueprintPure)
	float GetKilowattUsage() const;

	UFUNCTION(BlueprintPure)
	float GetOxygenReserve() const;

	UFUNCTION(BlueprintPure)
	float GetWaterStorage() const;

	virtual void BeginPlay() override;

private:

	virtual void SetupInitialResourceValues();

	UFUNCTION(BlueprintCallable)
	void SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)	void SetMoney(float NewValue = 0.0f);
	UFUNCTION(BlueprintCallable)	void SetKilowattUsage(float NewValue = 0.0f);
	UFUNCTION(BlueprintCallable)	void SetOxygenReserve(float NewValue = 0.0f);
	UFUNCTION(BlueprintCallable)	void SetWaterStorage(float NewValue = 0.0f);

public:

	UPROPERTY(BlueprintAssignable) FOnResourceUpdated OnResourceUpdated;

private:

	TMap<FGameplayTag, float> Resources;
};
