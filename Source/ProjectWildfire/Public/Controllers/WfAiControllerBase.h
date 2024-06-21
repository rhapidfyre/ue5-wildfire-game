// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WfAiControllerBase.generated.h"

UCLASS()
class PROJECTWILDFIRE_API AWfAiControllerBase : public AAIController
{
	GENERATED_BODY()

public:

	AWfAiControllerBase();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;
};
