// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfGlobalEnums.h"
#include "GameFramework/GameModeBase.h"

#include "WfGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDay, const ETimeOfDay&, TimeOfDayEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeasonChange, const EClimateSeason&, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChange, const EWeatherCondition&, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTemperatureChanged, const float&, TempAtSurface, const float&, TempAtAltitude);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameDateTimeUpdated, const FDateTime&, NewDateTime);

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:

	AWfGameModeBase();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Static Functions")
	static float ConvertFahrenheit(const float DegreesCelsius) { return (DegreesCelsius * (9 / 5)) + 32.0f; }

	UFUNCTION(BlueprintPure, Category = "Static Functions")
	static float ConvertCelsius(const float DegreesFahrenheit) { return (DegreesFahrenheit - 32) * (5 / 9); }

	UFUNCTION(BlueprintCallable, Category = "Game Mode Control")
	void ForceUpdate();

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetTemperatureSurface(const bool bUseCelsius) const;

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetTemperatureHigh(const bool bUseCelsius, const int DaysAhead = 0) const;

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetTemperatureLow(const bool bUseCelsius, const int DaysAhead = 0) const;

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetTemperatureAir(const bool bUseCelsius) const;

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	EWeatherCondition GetCurrentWeatherConditions() const { return CurrentWeather; }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	EClimateSeason GetCurrentSeason() const { return CurrentSeason; }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetRelativeHumidity() const { return WeatherHumidity; }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetDroughtFactor() const { return DroughtFactor; }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetAltitudeDifference() const { return AltitudeDiff; }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	ETimeOfDay GetTimeOfDayEvent() { return DetermineTimeOfDay(); }

	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	EAtmosphericStability GetAtmosphericStability() const { return CalcAtmosphericStability(); }

	// The speed of the wind (m/s)
	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetWindSpeed() const { return WeatherWindSpeed; }

	// Returns wind direction in radians (the direction FROM where the wind is coming from)
	UFUNCTION(BlueprintPure, Category = "Weather Conditions")
	float GetWindDirection() const { return WeatherWindSpeed; }


	UFUNCTION(BlueprintCallable)
	void SetTimeOfDayEvent(const ETimeOfDay& TimeOfDayEnum, const FTimespan& EventOccurence);

	UFUNCTION(BlueprintCallable, Category = "Game Time")
	const FDateTime& GetGameDateTime() const;

protected:
	virtual void BeginPlay() override;

	virtual void UpdateTimeOfDay(const FDateTime& NewDateTime);

	virtual void DetermineSeason();

	virtual void DetermineTemperature();

	virtual void DetermineWindSpeed();

	virtual void DetermineRelativeHumidity();

	virtual void DetermineDroughtFactor();

	virtual void DetermineWeatherCondition();

	virtual ETimeOfDay DetermineTimeOfDay();

private:

	UFUNCTION() void ClimateChangeTick();
	float CalcEnvironmentalLapseRate() const;
	EAtmosphericStability CalcAtmosphericStability() const;
	void SetCurrentSeason(const EClimateSeason& NewSeason);
	void SetCurrentWeather(const EWeatherCondition& NewCondition);
	void InitSeasonalTempAverages();
	void InitDailyTemperatureRange();
	void InitSeasonalDates();

public:

	// Allows overriding the starting date and time of the game
	// If set to zero (epoch start), it will use the current UTC time and date.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	FDateTime StartTimeOverride;

	// If true, the game will cycle seasons.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	bool bUseSeasons;

	// If true, the game will show values as metric (Celsius, Meters)
	// If false, values will be shown as imperial (Fahrenheit, Feet)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	bool bUseMetricSystem;

	// Upon the day of year (after Jan. 1) given, the season will change to the specified season.
	// If 'bUseSeasons' is false, the first season in the list will be permanent.
	// This should be left empty if all seasons are to be cycled, "campaign" style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	TMap<int32, EClimateSeason> SeasonalDates;

	// Defines the average temperatures for each season.
	// If empty, will be populated at game start.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	TMap<EClimateSeason, float> SeasonalTempAverages;

	// Defines the average temperate range (+/-) for each season.
	// If empty, will be populated at game start.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	TMap<EClimateSeason, float> SeasonalTempRanges;

	UPROPERTY(BlueprintAssignable)	FOnTimeOfDay OnTimeOfDay;
	UPROPERTY(BlueprintAssignable)  FOnSeasonChange OnSeasonChange;
	UPROPERTY(BlueprintAssignable)  FOnWeatherChange OnWeatherChange;
	UPROPERTY(BlueprintAssignable)  FOnTemperatureChanged OnTemperatureChanged;

private:

	// If this timer is valid, the game is ready
	FTimerHandle ClimateChange;

	FDateTime OriginDateTime; // Real-world time of when the save was created

	float SimulationSpeed;    // In-game seconds per real-world second

	TMap<ETimeOfDay, FTimespan> TimeOfDayEvents;

	float TemperatureSurface;		// Temp (°C) at ground level
	float TemperatureAltitude;		// Temp (°C) at highest altitude
	float AltitudeDiff;				// Difference between two points of measure (km)
	float WeatherAirStability;
	float WeatherWindSpeed;
	float WeatherHumidity;
	float DroughtFactor;

	FDateTime GameDateTime;
	EClimateSeason CurrentSeason;
	EWeatherCondition CurrentWeather;
};
