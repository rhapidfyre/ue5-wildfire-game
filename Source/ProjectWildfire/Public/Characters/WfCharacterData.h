// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"

#include "WfCharacterData.generated.h"


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FWfNamesStruct : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString NameValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool    bFeminine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool    bMasculine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float	PercentChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer EthnicGroups;
};
