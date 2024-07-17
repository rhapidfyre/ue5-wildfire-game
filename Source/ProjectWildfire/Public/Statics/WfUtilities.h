// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "WfUtilities.generated.h"


UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API UWfUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UWfUtilities();

	UFUNCTION(BlueprintPure, Category = "Utilities")
	static FString GenerateGUID();
};
