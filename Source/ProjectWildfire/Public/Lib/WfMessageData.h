// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums/WfMessageEnums.h"
#include "UObject/Object.h"
#include "WfMessageData.generated.h"

/**
 *
 */
USTRUCT(Blueprintable, BlueprintType)
struct PROJECTWILDFIRE_API FWfMessageData
{
	GENERATED_BODY()

	FWfMessageData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Data")
	UTexture2D* MessageIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Data")
	FString MessageTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Data")
	FString MessageBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Data")
	EMessageAlertType AlertLevel;

};
