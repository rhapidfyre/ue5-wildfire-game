// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfGameStateBase.h"

#include "Net/UnrealNetwork.h"
#include "Statics/WfGameModeBase.h"

AWfGameStateBase::AWfGameStateBase()
	: CheckTimeFrequency(1.0f)
	, GameModeBase(nullptr)
	, GameDateTime(FDateTime::MinValue())
{
}

void AWfGameStateBase::ForceUpdate()
{
	if (HasAuthority())
	{
		if (IsValid(GameModeBase))
		{
			GameModeBase->ForceUpdate();
		}
	}
	else
	{
		if (OnStateChangeGameTime.IsBound())
			OnStateChangeGameTime.Broadcast(GameDateTime);
		if (OnStateChangeTimeOfDay.IsBound())
			OnStateChangeTimeOfDay.Broadcast(GameTimeOfDay);
		if (OnStateChangeSeason.IsBound())
			OnStateChangeSeason.Broadcast(GameSeason);
		if (OnStateChangeWeather.IsBound())
			OnStateChangeWeather.Broadcast(GameWeatherCondition);
		if (OnStateChangeTemp.IsBound())
			OnStateChangeTemp.Broadcast(GameTemperatureSurface, GameTemperatureAir);
	}
}

USaveGame* AWfGameStateBase::CreateNewCharacter(const FGameplayTag& GetRole)
{
	AWfGameModeBase* GameMode = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameMode))
	{
		return GameMode->CreateNewCharacter(GetRole);
	}
	return nullptr;
}

void AWfGameStateBase::SetGameDateTime(const FDateTime& NewGameDateTime)
{
	if (HasAuthority())
	{
		GameDateTime = NewGameDateTime;
		if (OnStateChangeGameTime.IsBound())
			OnStateChangeGameTime.Broadcast(GameDateTime);
	}
}

const FDateTime& AWfGameStateBase::GetGameDateTime()
{
	UpdateGameDateTime();
	return GameDateTime;
}

const ETimeOfDay& AWfGameStateBase::GetTimeOfDay()
{
	return GameTimeOfDay;
}

const EClimateSeason& AWfGameStateBase::GetClimateSeason()
{
	return GameSeason;
}

const EWeatherCondition& AWfGameStateBase::GetWeatherCondition()
{
	return GameWeatherCondition;
}

const float AWfGameStateBase::GetTemperatureAir(bool InCelsius)
{
	return bUseCelsius ? GameTemperatureAir : GameModeBase->ConvertFahrenheit(GameTemperatureAir);
}

const float AWfGameStateBase::GetTemperatureSurface(bool InCelsius)
{
	return bUseCelsius ? GameTemperatureSurface : GameModeBase->ConvertFahrenheit(GameTemperatureSurface);
}

void AWfGameStateBase::UpdateGameDateTime()
{
	if (IsValid(GameModeBase))
		SetGameDateTime(GameModeBase->GetGameDateTime());

	// If the memory of the game mode is lost, reacquire it as soon as possible
	else
		GameModeBase = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
}

void AWfGameStateBase::UpdateTemperate_Implementation(const float& TempAtSurface, const float& TempAtAltitude)
{
	GameTemperatureSurface = TempAtSurface;
	GameTemperatureAir     = TempAtAltitude;
	if (OnStateChangeTemp.IsBound())
		OnStateChangeTemp.Broadcast(GameTemperatureSurface, GameTemperatureAir);
}

void AWfGameStateBase::UpdateTimeOfDay_Implementation(const ETimeOfDay& TimeOfDayEvent)
{
	GameTimeOfDay = TimeOfDayEvent;
	if (OnStateChangeTimeOfDay.IsBound())
		OnStateChangeTimeOfDay.Broadcast(GameTimeOfDay);
}

void AWfGameStateBase::UpdateSeason_Implementation(const EClimateSeason& NewSeason)
{
	GameSeason = NewSeason;
	if (OnStateChangeSeason.IsBound())
		OnStateChangeSeason.Broadcast(GameSeason);
}

void AWfGameStateBase::UpdateWeather_Implementation(const EWeatherCondition& NewCondition)
{
	GameWeatherCondition = NewCondition;
	if (OnStateChangeWeather.IsBound())
		OnStateChangeWeather.Broadcast(GameWeatherCondition);
}

void AWfGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		// Update the game time every n (CheckTimeFrequency) seconds
		FTimerDelegate GdtDelegate;
		GdtDelegate.BindUObject(this, &AWfGameStateBase::UpdateGameDateTime);
		GetWorldTimerManager().SetTimer(GameDateTimeChecker, GdtDelegate, CheckTimeFrequency, true);
	}

	if (IsValid(GameModeBase))
	{
		// Establish delegates
		GameModeBase->OnTemperatureChanged.AddDynamic(this, &AWfGameStateBase::UpdateTemperate);
		GameModeBase->OnTimeOfDay.AddDynamic(this, &AWfGameStateBase::UpdateTimeOfDay);
		GameModeBase->OnSeasonChange.AddDynamic(this, &AWfGameStateBase::UpdateSeason);
		GameModeBase->OnWeatherChange.AddDynamic(this, &AWfGameStateBase::UpdateWeather);
		UpdateTemperate(GameModeBase->GetTemperatureSurface(bUseCelsius), GameModeBase->GetTemperatureAir(bUseCelsius));
		UpdateTimeOfDay(GameModeBase->GetTimeOfDayEvent());
		UpdateSeason(GameModeBase->GetCurrentSeason());
		UpdateWeather(GameModeBase->GetCurrentWeatherConditions());

		SetGameDateTime(GameModeBase->GetGameDateTime());
	}
}

/** ************************
 *  REPLICATION / NETWORKING
 */

/**
 * \brief
 * \param OutLifetimeProps
 */
void AWfGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameDateTime, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameTimeOfDay, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameSeason, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameWeatherCondition, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameTemperatureSurface, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, GameTemperatureAir, COND_OwnerOnly);
}

void AWfGameStateBase::OnRep_GameDateTime_Implementation(const FDateTime& OldData)
{
	if (OnStateChangeGameTime.IsBound())
		OnStateChangeGameTime.Broadcast(GameDateTime);
}

void AWfGameStateBase::OnRep_GameTimeOfDay_Implementation(const ETimeOfDay& OldData)
{
	if (OnStateChangeTimeOfDay.IsBound())
		OnStateChangeTimeOfDay.Broadcast(GameTimeOfDay);
}

void AWfGameStateBase::OnRep_GameSeason_Implementation(const EClimateSeason& OldData)
{
	if (OnStateChangeSeason.IsBound())
		OnStateChangeSeason.Broadcast(GameSeason);
}

void AWfGameStateBase::OnRep_GameWeather_Implementation(const EWeatherCondition& OldData)
{
	if (OnStateChangeWeather.IsBound())
		OnStateChangeWeather.Broadcast(GameWeatherCondition);
}

void AWfGameStateBase::OnRep_GameTemp_Implementation(const float& OldData)
{
	if (OnStateChangeTemp.IsBound())
		OnStateChangeTemp.Broadcast(GameTemperatureSurface, GameTemperatureAir);
}

void AWfGameStateBase::OnRep_GameTempAir_Implementation(const float& OldData)
{
	if (OnStateChangeTemp.IsBound())
		OnStateChangeTemp.Broadcast(GameTemperatureSurface, GameTemperatureAir);
}
