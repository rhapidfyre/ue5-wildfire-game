// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "WfEquipmentData.generated.h"

UCLASS(BlueprintType)
class PROJECTWILDFIRE_API UEquipmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FName EquipmentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	UStaticMesh* Mesh;

};
