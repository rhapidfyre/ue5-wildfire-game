// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfPlayerStateBase.h"
#include "WfPlayerState.generated.h"

/**
 * A child of AWfPlayerStateBase that is used for Live gameplay.
 * Includes the logic for starting a new game, and reloading from a save.
 */
UCLASS()
class PROJECTWILDFIRE_API AWfPlayerState : public AWfPlayerStateBase
{
	GENERATED_BODY()

public:

	AWfPlayerState();

protected:

	virtual void BeginPlay() override;

private:

	FString SaveSlotName;

	int32 SaveUserIndex;

};
