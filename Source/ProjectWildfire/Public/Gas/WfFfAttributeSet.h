// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Gas/WfAttributeSet.h"

#include "WfFfAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Attributes applying to all Project Wildfire firefighting characters
 */
UCLASS(BlueprintType)
class PROJECTWILDFIRE_API UWfFfAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UWfFfAttributeSet();

protected:

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
