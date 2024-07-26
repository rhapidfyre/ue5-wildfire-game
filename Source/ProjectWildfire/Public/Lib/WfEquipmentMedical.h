// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Lib/WfEquipmentData.h"

#include "WfEquipmentMedical.generated.h"


UCLASS(BlueprintType)
class PROJECTWILDFIRE_API UEquipmentMedicalData : public UEquipmentDataAsset
{
	GENERATED_BODY()

public:

	// Specific properties for medical equipment
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Medical Equipment")
	float ExpirationDate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Medical Equipment")
	float SterilityLevel;

	// Add specific properties for medical equipment here
};
