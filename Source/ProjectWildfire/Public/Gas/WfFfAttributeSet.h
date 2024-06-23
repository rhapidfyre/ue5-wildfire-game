// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WfFfAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS(BlueprintType)
class PROJECTWILDFIRE_API UWfFfAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UWfFfAttributeSet();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Health, Category = "Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UWfFfAttributeSet, Health)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Fatigue, Category = "Attributes")
    FGameplayAttributeData Fatigue;
    ATTRIBUTE_ACCESSORS(UWfFfAttributeSet, Fatigue)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Morale, Category = "Attributes")
    FGameplayAttributeData Morale;
    ATTRIBUTE_ACCESSORS(UWfFfAttributeSet, Morale)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Thirst, Category = "Attributes")
    FGameplayAttributeData Thirst;
    ATTRIBUTE_ACCESSORS(UWfFfAttributeSet, Thirst)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_Hunger, Category = "Attributes")
    FGameplayAttributeData Hunger;
    ATTRIBUTE_ACCESSORS(UWfFfAttributeSet, Hunger)

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
