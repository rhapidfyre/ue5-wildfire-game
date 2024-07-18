// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WfCharacterBase.h"

#include "WfVehicleData.generated.h"


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FVehicleSeat
{
	GENERATED_BODY()

	FVehicleSeat();
	explicit FVehicleSeat(int NewSeatNumber);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Seats")
	int SeatNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Seats")
	AWfCharacterBase* SeatOccupant;
};
