// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAiDrivingBehavior : uint8
{
	// Driver just drives aimlessly, abiding by all laws
	None = 0		UMETA(DisplayName = "Casual Drive"),

	// Driver rolls stop signs, accelerating quickly, braking last second
	Rushed			UMETA(DisplayName = "Late for Work"),

	// Drives within legal limits but is slow to react (cell phone)
	Distracted		UMETA(DisplayName = "Distracted"),

	// Slow reactions, tailgating
	Intoxicated		UMETA(DisplayName = "Intoxicated"),

	// Abides by all laws, with perfect reaction times
	Professional	UMETA(DisplayName = "Professional")
};

UENUM(BlueprintType)
enum class EResponseCode : uint8
{
	// Don't drive, avoid vehicles.
	None = 0		UMETA(DisplayName = "No Change"),

	// Drive casually as part of normal traffic
	One				UMETA(DisplayName = "Drive Casually"),

	// Disregard basic laws to get to goal quicker
	Two				UMETA(DisplayName = "Drive With Urgency"),

	// Drive with lights and sirens to goal as fast as possible
	Three			UMETA(DisplayName = "Emergency Operations"),
};

UENUM(BlueprintType)
enum class EAiDrivingState : uint8
{
	// Don't drive, avoid vehicles.
	None = 0		UMETA(DisplayName = "Avoid Vehicle Use"),

	// Get in the vehicle and then drive to the goal destination
	Casual			UMETA(DisplayName = "Drive Without a Goal"),

	// Use the vehicle and then drive to the goal destination
	Goal			UMETA(DisplayName = "Drive Towards Goal"),
};
