// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfPlayerStateBase.h"
#include "WfTestPlayerState.generated.h"

/**
 * A child of AWfPlayerStateBase that is used for development/test gameplay
 */
UCLASS()
class PROJECTWILDFIRE_API AWfTestPlayerState : public AWfPlayerStateBase
{
	GENERATED_BODY()

public:

	AWfTestPlayerState();

protected:

	virtual void BeginPlay() override;
};
