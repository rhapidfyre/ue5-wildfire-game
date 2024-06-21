// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WfGlobalEnums.generated.h"


UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Midnight = 0,
	Noon,
	Sunrise,
	Sunset,
	Moonrise,
	Moonset,
	Daylight,
	Twilight,
	Dusk,
	HighNoon,
	Nightfall,
};

UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
	Clear = 0,
	Overcast,
	Cloudy,
	Fog,
	DenseFog,
	Rain,
	Thunderstorm
};

UENUM(BlueprintType)
enum class EClimateSeason : uint8
{
	EarlyWinter = 0,
	MidWinter,
	LateWinter,
	EarlySpring,
	LateSpring,
	EarlySummer,
	MidSummer,
	LateSummer,
	EarlyAutumn,
	MidAutumn,
	LateAutumn,
};

UENUM(BlueprintType)
enum class EAtmosphericStability : uint8
{
	Stable = 0,
	Unstable,
	Conditional
};
