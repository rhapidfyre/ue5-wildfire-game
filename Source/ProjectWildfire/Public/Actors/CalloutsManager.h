// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalloutsManager.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API ACalloutsManager : public AActor
{
	GENERATED_BODY()

public:

	ACalloutsManager();

	static ACalloutsManager* GetInstance(UWorld* World);


	virtual void BeginPlay() override;

	void GenerateCallout();

private:
	static ACalloutsManager* Instance;
	FTimerHandle CalloutTimerHandle;
};
