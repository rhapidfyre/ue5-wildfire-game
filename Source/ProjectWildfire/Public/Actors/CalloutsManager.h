// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CalloutsManager.generated.h"

struct FCalloutData;
struct FCallouts;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalloutExpired, const FCalloutData&, CalloutData);

UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API ACalloutsManager : public AActor
{
	GENERATED_BODY()

public:

	ACalloutsManager();

	UFUNCTION(BlueprintCallable, Category = "Callouts Manager Singleton", meta = (WorldContext = "WorldContextObject"))
	static ACalloutsManager* GetInstance(UObject* WorldContext);

	UFUNCTION(BlueprintCallable) void ForceCallout(const FName& CalloutType);
	UFUNCTION(BlueprintCallable) void GenerateCallout();

	void Multicast_CalloutExpired(const FCalloutData& CalloutData);


	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ForceCallout(const FName& CalloutType);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Pre-blueprint interception of event creation (prior to 'CreateCallout()')
	UFUNCTION(BlueprintNativeEvent)
	void NewCallout(const FName& CalloutType, FCalloutData& CalloutData);

	// Creates the callout actor & data
	virtual void CreateCallout(const FName& CalloutType, FCalloutData& CalloutData);

public:

	UPROPERTY(BlueprintAssignable) FOnCalloutExpired OnCalloutExpired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Callouts Manager")
	float SecondsToRespond = 30.0f;

private:

	FCallouts GetCalloutDataRow(const FName& CalloutType) const;

	static ACalloutsManager* Instance;
	FTimerHandle CalloutTimerHandle;
};
