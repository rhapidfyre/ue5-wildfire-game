// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WfGlobalEnums.h"
#include "Characters/WfCharacterBase.h"
#include "Delegates/Delegate.h"
#include "GameFramework/GameStateBase.h"
#include "Saves/WfCharacterSaveGame.h"

#include "WfGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeTimeOfDay, const ETimeOfDay&, TimeOfDayEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeSeason, const EClimateSeason&, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeWeather, const EWeatherCondition&, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChangeTemp, const float, TempAtSurface, const float, TempAtAltitude);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChangeGameTime, const FDateTime&, NewDateTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransferListUpdated, const FJobContractData&, ContractId, const bool, bIsAvailable);


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

	UFUNCTION(BlueprintPure)
	TArray<FJobContractData>& GetJobContracts();

	UFUNCTION()
	void JobContractOffered(const FJobContractData& JobContract);

	UFUNCTION()
	void JobContractExpired(const FJobContractData& JobContract);

	UFUNCTION(BlueprintCallable)
	void JobContractRemove(const FJobContractData& JobContract, bool bDeleteSave = true);

	UFUNCTION(BlueprintCallable)
	void JobContractRemoveById(const FString& ContractId, bool bDeleteSave = true);

	UFUNCTION(BlueprintCallable)
	FJobContractData GetJobContract(const FString& ContractId) const;

	UFUNCTION(BlueprintCallable) void ForceUpdate();

	UFUNCTION(BlueprintCallable) USaveGame* CreateNewCharacter(const FGameplayTag& GetRole);

	UFUNCTION(BlueprintPure)
	TArray<UWfFirefighterSaveGame*> GetTransferList() const;

	void SetGameDateTime(const FDateTime& NewGameDateTime);

	UFUNCTION(BlueprintCallable, Category = "Simulated Environment")
	const FDateTime& GetGameDateTime();

	UFUNCTION(BlueprintPure, Category = "Simulated Environment")
	const ETimeOfDay& GetTimeOfDay();

	UFUNCTION(BlueprintPure, Category = "Simulated Environment")
	const EClimateSeason& GetClimateSeason();

	UFUNCTION(BlueprintPure, Category = "Simulated Environment")
	const EWeatherCondition& GetWeatherCondition();

	UFUNCTION(BlueprintPure, Category = "Simulated Environment")
	float GetTemperatureAir(bool InCelsius = true);

	UFUNCTION(BlueprintPure, Category = "Simulated Environment")
	float GetTemperatureSurface(bool InCelsius = true);

	UFUNCTION(BlueprintNativeEvent)	void UpdateTemperate(const float TempAtSurface, const float TempAtAltitude); // Triggered when the gamemode announces a change to the temperature
	UFUNCTION(BlueprintNativeEvent)	void UpdateTimeOfDay(const ETimeOfDay& TimeOfDayEvent); // Triggered when the gamemode announces a time of day change
	UFUNCTION(BlueprintNativeEvent)	void UpdateSeason(const EClimateSeason& NewSeason);    // Triggered when the gamemode announces a change in seasons
	UFUNCTION(BlueprintNativeEvent)	void UpdateWeather(const EWeatherCondition& NewCondition);   // Triggered when the gamemode announces a change to the weather

	UFUNCTION(BlueprintCallable)
	UWfFirefighterSaveGame* GetSaveGameFromJobContract(const FJobContractData& JobContract);

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(Client, Reliable) void OnRep_GameDateTime(const FDateTime& OldData);
	UFUNCTION(Client, Reliable) void OnRep_GameTimeOfDay(const ETimeOfDay& OldData);
	UFUNCTION(Client, Reliable) void OnRep_GameSeason(const EClimateSeason& OldData);
	UFUNCTION(Client, Reliable) void OnRep_GameWeather(const EWeatherCondition& OldData);
	UFUNCTION(Client, Reliable) void OnRep_GameTemp(const float& OldData);
	UFUNCTION(Client, Reliable) void OnRep_GameTempAir(const float& OldData);
	UFUNCTION(Client, Reliable) void OnRep_HiringList(const TArray<FJobContractData>& OldHiringList);

	// Called by the GS Clock to keep the time updated within a set interval
	UFUNCTION()	void UpdateGameDateTime();

public:

	UPROPERTY(BlueprintAssignable) FOnStateChangeTimeOfDay  OnStateChangeTimeOfDay;
	UPROPERTY(BlueprintAssignable) FOnStateChangeSeason		OnStateChangeSeason;
	UPROPERTY(BlueprintAssignable) FOnStateChangeWeather    OnStateChangeWeather;
	UPROPERTY(BlueprintAssignable) FOnStateChangeTemp		OnStateChangeTemp;
	UPROPERTY(BlueprintAssignable) FOnStateChangeGameTime   OnStateChangeGameTime;
	UPROPERTY(BlueprintAssignable) FOnTransferListUpdated   OnTransferListUpdated;

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

	UPROPERTY(ReplicatedUsing = OnRep_HiringList)    TArray<FJobContractData> HiringList;
	UPROPERTY(ReplicatedUsing = OnRep_GameDateTime)  FDateTime			GameDateTime;
	UPROPERTY(ReplicatedUsing = OnRep_GameTimeOfDay) ETimeOfDay			GameTimeOfDay;
	UPROPERTY(ReplicatedUsing = OnRep_GameSeason)	 EClimateSeason		GameSeason;
	UPROPERTY(ReplicatedUsing = OnRep_GameWeather)   EWeatherCondition	GameWeatherCondition;
	UPROPERTY(ReplicatedUsing = OnRep_GameTemp)		 float				GameTemperatureSurface;
	UPROPERTY(ReplicatedUsing = OnRep_GameTempAir)   float				GameTemperatureAir;
};
