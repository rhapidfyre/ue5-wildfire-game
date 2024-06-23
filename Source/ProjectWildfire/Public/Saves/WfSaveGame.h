// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "WfSaveGame.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API UWfSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UWfSaveGame() : SaveSlotName("Untitled"), SaveSlotIndex(0) {}

	FString SaveSlotName;

	int SaveSlotIndex;
};
