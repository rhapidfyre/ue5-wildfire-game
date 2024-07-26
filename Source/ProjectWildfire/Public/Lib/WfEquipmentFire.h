// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Lib/WfEquipmentData.h"

#include "WfEquipmentFire.generated.h"


UCLASS(BlueprintType)
class PROJECTWILDFIRE_API UEquipmentDataFfAsset : public UEquipmentDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firefighting Equipment")
	float Durability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firefighting Equipment")
	float Weight;

};
