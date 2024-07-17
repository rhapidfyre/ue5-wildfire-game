// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Delegates/Delegate.h"

#include "WfAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeUpdated,
	const FGameplayAttribute&, GameAttribute, const float, OldValue,   const float, NewValue );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,   const float,    OldValue,   const float,    NewValue );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFatigueChanged,  const float,    OldValue,   const float,    NewValue );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMoraleChanged,   const float,    OldValue,   const float,    NewValue );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnThirstChanged,   const float,    OldValue,   const float,    NewValue );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHungerChanged,   const float,    OldValue,   const float,    NewValue );

/**
 * Attributes applying to all Project Wildfire characters
 */
UCLASS()
class PROJECTWILDFIRE_API UWfAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UWfAttributeSet();

	bool SetAttributeValue(const FGameplayAttribute&  GameAttribute, float NewValue);
	float ModifyAttribute(const FGameplayAttribute&  GameAttribute, float AddValue);
	float ModifyAttributePercent(const FGameplayAttribute&  GameAttribute, float PercentChange);
	float GetAttributeValue(const FGameplayAttribute&  GameAttribute) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UWfAttributeSet, Health)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Fatigue, Category = "Attributes")
	FGameplayAttributeData Fatigue;
	ATTRIBUTE_ACCESSORS(UWfAttributeSet, Fatigue)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Morale, Category = "Attributes")
	FGameplayAttributeData Morale;
	ATTRIBUTE_ACCESSORS(UWfAttributeSet, Morale)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Thirst, Category = "Attributes")
	FGameplayAttributeData Thirst;
	ATTRIBUTE_ACCESSORS(UWfAttributeSet, Thirst)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Hunger, Category = "Attributes")
	FGameplayAttributeData Hunger;
	ATTRIBUTE_ACCESSORS(UWfAttributeSet, Hunger)


	UPROPERTY(BlueprintAssignable)
	FOnAttributeUpdated OnAttributeUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFatigueChanged OnFatigueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnMoraleChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnThirstChanged;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHungerChanged;

protected:

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_Fatigue(const FGameplayAttributeData& OldFatigue);

	UFUNCTION()
	virtual void OnRep_Morale(const FGameplayAttributeData& OldMorale);

	UFUNCTION()
	virtual void OnRep_Thirst(const FGameplayAttributeData& OldThirst);

	UFUNCTION()
	virtual void OnRep_Hunger(const FGameplayAttributeData& OldHunger);

};
