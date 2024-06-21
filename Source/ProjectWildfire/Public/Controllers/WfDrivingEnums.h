// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAiDrivingBehavior : uint8
{
	// Driver just drives aimlessly, abiding by all laws
	None = 0		UMETA(DisplayName = "Basic Driver"),

	// Driver rolls stop signs, accelerating quickly, braking last second
	Rushed			UMETA(DisplayName = "Late for Task"),

	// Drives within legal limits but is slow to react (cell phone)
	Distracted		UMETA(DisplayName = "Distracted Driver"),

	// Slow reactions, tailgating
	Intoxicated		UMETA(DisplayName = "Intoxicated Driver"),

	// Abides by all laws, with perfect reaction times
	Professional	UMETA(DisplayName = "Professional Driver")
};

UENUM(BlueprintType)
enum class EAiDrivingMethod : uint8
{
	None = 0		UMETA(DisplayName = "Basic"),

	// Driver rolls stop signs, accelerating quickly, braking last second
	Urgent			UMETA(DisplayName = "Late for Task"),

	// Drives within legal limits but is slow to react (cell phone)
	Emergency		UMETA(DisplayName = "Distracted Driver")
};
