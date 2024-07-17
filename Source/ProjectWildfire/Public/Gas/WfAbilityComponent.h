// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Delegates/Delegate.h"

#include "WfAbilityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged,
	const FGameplayAttribute&, GameAttribute, const float, OldValue,   const float, NewValue );


class UWfAttributeSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class PROJECTWILDFIRE_API UWfAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UWfAbilityComponent();

	void InitializeAttributes(UWfAttributeSet* AttributeSet);

	virtual void TimerTick();

	void SetNewTimerRate(const float NewTimeRate = 1.0f);

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetNewAttributeValue(const FGameplayAttribute& GameAttribute, const float NewValue);
	virtual void ModifyAttributePercent(const FGameplayAttribute& GameAttribute, const float PercentChange);

	UFUNCTION(BlueprintCallable) float SetHunger(float NewValue);
	UFUNCTION(BlueprintCallable) float ModifyHunger(float PercentChange);
	UFUNCTION(BlueprintCallable) float AddHunger(float ModValue);
	UFUNCTION(BlueprintCallable) float RemoveHunger(float ModValue);

	UFUNCTION(BlueprintCallable) float ModifyThirst(float PercentChange);
	UFUNCTION(BlueprintCallable) float SetThirst(float NewValue);
	UFUNCTION(BlueprintCallable) float AddThirst(float ModValue);
	UFUNCTION(BlueprintCallable) float RemoveThirst(float ModValue);

public:

	UFUNCTION()
	void AttributeUpdated(const FGameplayAttribute& GameAttribute,
						  const float OldValue, const float NewValue);

	UPROPERTY(BlueprintAssignable) FOnAttributeChanged OnAttributeChanged;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) float StatReduction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float ReductionFatigue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float ReductionMorale;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float ReductionThirst;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float ReductionHunger;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) float AttributeTimeRate;

private:

	FTimerHandle AttributeTimer;

	UPROPERTY() UWfAttributeSet* CharacterAttributes;

};
