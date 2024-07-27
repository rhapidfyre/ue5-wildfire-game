// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Statics/WfGlobalData.h"

#include "WfPropertyActor.generated.h"


UCLASS(BlueprintType)
class PROJECTWILDFIRE_API AWfPropertyActor : public AActor
{
	GENERATED_BODY()

public:

	AWfPropertyActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure) FStreetAddress GetStreetAddress() const;
	UFUNCTION(BlueprintPure) FString GetStreetAddressAsString() const;
	UFUNCTION(BlueprintPure) FString GetStreetAddressForVox() const;

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void DetermineAddress();

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* PropertyCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBoxComponent* PropertyArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Estate")
	FGameplayTag PropertyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Real Estate")
	FStreetAddress PropertyAddress;
};
