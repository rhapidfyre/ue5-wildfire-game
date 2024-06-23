// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WfGlobalEnums.h"
#include "Delegates/Delegate.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SaveGame.h"

#include "WfGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeTimeOfDay, const ETimeOfDay&, TimeOfDayEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeSeason, const EClimateSeason&, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeWeather, const EWeatherCondition&, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChangeTemp, const float&, TempAtSurface, const float&, TempAtAltitude);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeGameTime, const FDateTime&, NewDateTime);


class AWfGameModeBase;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:

	AWfGameStateBase();

	UFUNCTION(BlueprintCallable) void ForceUpdate();

	UFUNCTION(BlueprintCallable) USaveGame* CreateNewCharacter(const FGameplayTag& GetRole);

	void SetGameDateTime(const FDateTime& NewGameDateTime);

	UFUNCTION(BlueprintCallable, Category = "Simulated Environment") const FDateTime& GetGameDateTime();
	UFUNCTION(BlueprintPure, Category = "Simulated Environment") const ETimeOfDay& GetTimeOfDay();
	UFUNCTION(BlueprintPure, Category = "Simulated Environment") const EClimateSeason& GetClimateSeason();
	UFUNCTION(BlueprintPure, Category = "Simulated Environment") const EWeatherCondition& GetWeatherCondition();
	UFUNCTION(BlueprintPure, Category = "Simulated Environment") const float GetTemperatureAir(bool InCelsius = true);
	UFUNCTION(BlueprintPure, Category = "Simulated Environment") const float GetTemperatureSurface(bool InCelsius = true);

	UFUNCTION(BlueprintNativeEvent)	void UpdateTemperate(const float& TempAtSurface, const float& TempAtAltitude); // Triggered when the gamemode announces a change to the temperature
	UFUNCTION(BlueprintNativeEvent)	void UpdateTimeOfDay(const ETimeOfDay& TimeOfDayEvent); // Triggered when the gamemode announces a time of day change
	UFUNCTION(BlueprintNativeEvent)	void UpdateSeason(const EClimateSeason& NewSeason);    // Triggered when the gamemode announces a change in seasons
	UFUNCTION(BlueprintNativeEvent)	void UpdateWeather(const EWeatherCondition& NewCondition);   // Triggered when the gamemode announces a change to the weather

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(Client, Unreliable) void OnRep_GameDateTime(const FDateTime& OldData);
	UFUNCTION(Client, Unreliable) void OnRep_GameTimeOfDay(const ETimeOfDay& OldData);
	UFUNCTION(Client, Unreliable) void OnRep_GameSeason(const EClimateSeason& OldData);
	UFUNCTION(Client, Unreliable) void OnRep_GameWeather(const EWeatherCondition& OldData);
	UFUNCTION(Client, Unreliable) void OnRep_GameTemp(const float& OldData);
	UFUNCTION(Client, Unreliable) void OnRep_GameTempAir(const float& OldData);

	// Called by the GS Clock to keep the time updated within a set interval
	UFUNCTION()	void UpdateGameDateTime();


public:


	UPROPERTY(BlueprintAssignable) FOnStateChangeTimeOfDay  OnStateChangeTimeOfDay;
	UPROPERTY(BlueprintAssignable) FOnStateChangeSeason		OnStateChangeSeason;
	UPROPERTY(BlueprintAssignable) FOnStateChangeWeather    OnStateChangeWeather;
	UPROPERTY(BlueprintAssignable) FOnStateChangeTemp		OnStateChangeTemp;
	UPROPERTY(BlueprintAssignable) FOnStateChangeGameTime   OnStateChangeGameTime;

	// The frequency (in seconds) to check for an update to the GameDateTime value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	float CheckTimeFrequency;

	// The frequency (in seconds) to check for an update to the GameDateTime value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulated Game Time")
	bool bUseCelsius;

private:

	// A set interval to update the client's game time awareness
	// Defaults to once per real-world second (1.0f)
	FTimerHandle GameDateTimeChecker;
	UPROPERTY()	AWfGameModeBase* GameModeBase;

	UPROPERTY(ReplicatedUsing = OnRep_GameDateTime)  FDateTime			GameDateTime;
	UPROPERTY(ReplicatedUsing = OnRep_GameTimeOfDay) ETimeOfDay			GameTimeOfDay;
	UPROPERTY(ReplicatedUsing = OnRep_GameSeason)	 EClimateSeason		GameSeason;
	UPROPERTY(ReplicatedUsing = OnRep_GameWeather)   EWeatherCondition	GameWeatherCondition;
	UPROPERTY(ReplicatedUsing = OnRep_GameTemp)		 float				GameTemperatureSurface;
	UPROPERTY(ReplicatedUsing = OnRep_GameTempAir)   float				GameTemperatureAir;
};
