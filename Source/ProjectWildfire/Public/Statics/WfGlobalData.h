// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "WfGlobalData.generated.h"


class AWfVehicleBase;

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FResourceNeeds
{
	GENERATED_BODY()
	// If true, the call/scenario cannot be accepted without this resource
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address") bool bIsRequired;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address") TSubclassOf<AWfVehicleBase> ResourceType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address") int MinimumQuantity;
};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FStreetAddress
{
	GENERATED_BODY()

	FStreetAddress();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address")
	int BlockNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address")
	int HouseNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address")
	int SuiteNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address")
	FName StreetName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Street Address")
	FName StreetType;
};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FVoxSounds : public FTableRowBase
{
	GENERATED_BODY()

	FVoxSounds();

	// The primary sound to play when this phrase is chosen
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox")
	USoundBase* VoxSound;

	// Sounds that can be used as an alternative to the primary sound
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox")
	TArray<USoundBase*> VoxAlternatives;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Vox")
	int VoxVoice;
};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FShiftSchedule
{
	GENERATED_BODY()

	FShiftSchedule();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Shift Schedule") int32 DayOfWeek;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Shift Schedule") FTimespan StartTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Shift Schedule") FTimespan Duration;
};


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCallouts : public FTableRowBase
{
	GENERATED_BODY()

	FCallouts();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 AlertLevel;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FName VoxPhrase;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float Payment;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float Penalty;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FTimespan TimeToRespond;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") TMap< TSubclassOf<AWfVehicleBase>, int > MinimumUnits;
};
