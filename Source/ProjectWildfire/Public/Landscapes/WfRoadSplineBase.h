// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineActor.h"

#include "WfRoadSplineBase.generated.h"


UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API AWfRoadSplineBase : public AActor
{
	GENERATED_BODY()

public:

	AWfRoadSplineBase();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;
};
