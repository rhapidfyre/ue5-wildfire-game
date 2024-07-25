// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WfMessageEnums.generated.h"

/**
 *
 */
UENUM(Blueprintable, BlueprintType)
enum class EMessageAlertType : uint8
{
    None = 0,
	Notice,
	Warning,
	Error,
	Critical
};
