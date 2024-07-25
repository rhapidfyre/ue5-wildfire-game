// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Statics/WfGlobalData.h"
#include "CalloutsManager.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API ACalloutsManager : public AActor
{
	GENERATED_BODY()

public:

	ACalloutsManager();

	UFUNCTION(BlueprintCallable, Category = "Callouts Manager", meta = (WorldContext = "WorldContextObject"))
	static ACalloutsManager* GetInstance(UObject* WorldContext);

	UFUNCTION(BlueprintCallable) void ForceCallout(const FName& CalloutType);
	UFUNCTION(BlueprintCallable) void GenerateCallout();


	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ForceCallout(const FName& CalloutType);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FCallouts GetCalloutData(const FName& CalloutType) const;

	static ACalloutsManager* Instance;
	FTimerHandle CalloutTimerHandle;
};
