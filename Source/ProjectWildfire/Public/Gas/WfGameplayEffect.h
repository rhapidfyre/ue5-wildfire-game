// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "WfGameplayEffect.generated.h"

/**
 *
 */
UCLASS()
class PROJECTWILDFIRE_API UWfGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:

	UWfGameplayEffect();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture2D* EffectIcon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString DisplayName;

};
