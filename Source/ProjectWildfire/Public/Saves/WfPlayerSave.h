// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfSaveGame.h"
#include "WfPlayerSave.generated.h"

struct FJobContractData;
/**
 *
 */
UCLASS()
class PROJECTWILDFIRE_API UWfPlayerSave : public UWfSaveGame
{
    GENERATED_BODY()
public:
	TArray<FJobContractData> SavedPersonnel;
};
