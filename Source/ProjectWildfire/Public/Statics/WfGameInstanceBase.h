// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "WfGameInstanceBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API UWfGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()

public:

	UWfGameInstanceBase();

};
