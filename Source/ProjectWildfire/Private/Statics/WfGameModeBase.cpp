// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfGameModeBase.h"

#include "Logging/StructuredLog.h"

AWfGameModeBase::AWfGameModeBase()
    : TemperatureSurface(24.0f),
      TemperatureAltitude(24.0f),
      AltitudeDiff(1),
      WeatherAirStability(0),
      WeatherWindSpeed(0),
      WeatherHumidity(0),
      DroughtFactor(0),
      CurrentSeason(EClimateSeason::EarlySpring),
      CurrentWeather(EWeatherCondition::Clear)
{
    SimulationSpeed = 180.0f; // 1 real second = 'n' game seconds
}

void AWfGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AWfGameModeBase::ForceUpdate()
{
    DetermineSeason();
    DetermineWeatherCondition();
    DetermineTemperature();
    DetermineDroughtFactor();
    DetermineRelativeHumidity();
    DetermineWindSpeed();
}

// Returns the current surface temperature
float AWfGameModeBase::GetTemperatureSurface(const bool bUseCelsius) const
{
    return bUseCelsius ? TemperatureSurface : ConvertFahrenheit(TemperatureSurface);
}

float AWfGameModeBase::GetTemperatureHigh(const bool bUseCelsius, const int DaysAhead) const
{
    const float AvgTemp = SeasonalTempAverages.Contains(CurrentSeason)
                        ? SeasonalTempAverages[CurrentSeason] : 15.0f;

    const float TempRange = SeasonalTempRanges.Contains(CurrentSeason)
                            ? SeasonalTempRanges[CurrentSeason] : 10.0f;
    float Temp = AvgTemp;

    for (int i = 5; i < 15; ++i)
    {
        Temp += (TempRange / 2.0f) * FMath::Sin(1.0f * PI);
    }
    return bUseCelsius ? Temp : ConvertFahrenheit(Temp);
}

float AWfGameModeBase::GetTemperatureLow(const bool bUseCelsius, const int DaysAhead) const
{
    const float AvgTemp = SeasonalTempAverages.Contains(CurrentSeason)
                        ? SeasonalTempAverages[CurrentSeason] : 15.0f;

    const float TempRange = SeasonalTempRanges.Contains(CurrentSeason)
                            ? SeasonalTempRanges[CurrentSeason] : 10.0f;

    float Temp = AvgTemp;

    for (int i = 1; i < 14; ++i)
    {
        Temp -= (TempRange / 2.0f) * FMath::Sin(((i - 15) / 4) * PI);
    }
    return bUseCelsius ? Temp : ConvertFahrenheit(Temp);
}

// Returns the current air temperature
float AWfGameModeBase::GetTemperatureAir(const bool bUseCelsius) const
{
    return bUseCelsius ? TemperatureAltitude : ConvertFahrenheit(TemperatureAltitude);
}


void AWfGameModeBase::SetTimeOfDayEvent(const ETimeOfDay& TimeOfDayEnum, const FTimespan& EventOccurence)
{
    TimeOfDayEvents.Add(TimeOfDayEnum, EventOccurence);
}

const FDateTime& AWfGameModeBase::GetGameDateTime() const
{
    return GameDateTime;
}

void AWfGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    if (SeasonalDates.IsEmpty())
        InitSeasonalDates();

    FDateTime StartingTime = FDateTime::UtcNow();
    if (StartTimeOverride > FDateTime::MinValue())
        StartingTime = StartTimeOverride;

    GameDateTime   = StartingTime;
    OriginDateTime = StartingTime;

    SetCurrentWeather(EWeatherCondition::Clear);

    InitDailyTemperatureRange();
    InitSeasonalTempAverages();

    SetTimeOfDayEvent(ETimeOfDay::Daylight, FTimespan(6,30,0));
    SetTimeOfDayEvent(ETimeOfDay::Nightfall, FTimespan(21,00,0));
    SetTimeOfDayEvent(ETimeOfDay::Midnight, FTimespan(0,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Noon, FTimespan(12,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Sunrise, FTimespan(6,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Sunset, FTimespan(19,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Moonrise, FTimespan(20,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Moonset, FTimespan(4,0,0));
    SetTimeOfDayEvent(ETimeOfDay::Twilight, FTimespan(5,30,0));
    SetTimeOfDayEvent(ETimeOfDay::Dusk, FTimespan(19,30,0));
    SetTimeOfDayEvent(ETimeOfDay::HighNoon, FTimespan(14,0,0));

    if (OnTimeOfDay.IsBound())
        OnTimeOfDay.Broadcast(DetermineTimeOfDay());

    FTimerDelegate ClimateDelegate;
    ClimateDelegate.BindUObject(this, &AWfGameModeBase::ClimateChangeTick);
    GetWorldTimerManager().SetTimer(ClimateChange, ClimateDelegate, 1.0f, true);
}

float AWfGameModeBase::CalcEnvironmentalLapseRate() const
{
    // Calculate the environmental lapse rate (ELR)
    const float Divisor = FMath::IsNearlyZero(AltitudeDiff) ? 1.0f : AltitudeDiff;
    return (TemperatureSurface - TemperatureAltitude) / Divisor;
}

EAtmosphericStability AWfGameModeBase::CalcAtmosphericStability() const
{
    constexpr float DryLapseRate   = 9.8f; // Dry Adiabatic Lapse Rate (°C/km)
    constexpr float MoistLapseRate = 6.0f; // Moist Adiabatic Lapse Rate (°C/km)

    const float EnvLapseRate = CalcEnvironmentalLapseRate();

    if (EnvLapseRate > DryLapseRate)
        return EAtmosphericStability::Unstable;
    if (EnvLapseRate < MoistLapseRate)
        return EAtmosphericStability::Stable;
    return EAtmosphericStability::Conditional;
}

void AWfGameModeBase::SetCurrentSeason(const EClimateSeason& NewSeason)
{
    CurrentSeason = NewSeason;
    if (OnSeasonChange.IsBound())
        OnSeasonChange.Broadcast(NewSeason);
}

void AWfGameModeBase::SetCurrentWeather(const EWeatherCondition& NewCondition)
{
    CurrentWeather = NewCondition;
    if (OnWeatherChange.IsBound())
        OnWeatherChange.Broadcast(NewCondition);
}

void AWfGameModeBase::ClimateChangeTick()
{
    // Updates the time, determines hourly temperature, and triggers delegates
    UpdateTimeOfDay(GameDateTime + FTimespan::FromSeconds(SimulationSpeed));

    if (bUseSeasons)
        DetermineSeason();

    DetermineWindSpeed();
    DetermineRelativeHumidity();
    DetermineDroughtFactor();

}

/**
 * \brief Updates the current date and time, broadcasting to the OnTimeOfDay event.
 * \param NewDateTime The new date time to replace the current date time with
 */
void AWfGameModeBase::UpdateTimeOfDay(const FDateTime& NewDateTime)
{
    const FDateTime OldDateTime = GameDateTime;
    GameDateTime = NewDateTime;
    if (!OnTimeOfDay.IsBound()) return;

    const FTimespan OldTimeOfDate = OldDateTime.GetTimeOfDay();
    const FTimespan NowTimeOfDate = NewDateTime.GetTimeOfDay();

    // If the day has changed, broadcast midnight
    if (OldDateTime.GetDay() != GameDateTime.GetDay())
    {
        UE_LOGFMT(LogTemp, Warning, "Time of Day Event = '{EventName}'",
            UEnum::GetValueAsString(ETimeOfDay::Midnight));
        OnTimeOfDay.Broadcast(ETimeOfDay::Midnight);
    }
    else
    {
        // If the hour has changed, so has the temperature
        if (OldDateTime.GetHour() < NewDateTime.GetHour())
        {
            DetermineTemperature();
        }

        // Loop through the time of day events, triggering them as necessary
        for (const auto& TimeOfDayEvent : TimeOfDayEvents)
        {
            if (OldTimeOfDate < TimeOfDayEvent.Value && NowTimeOfDate >= TimeOfDayEvent.Value)
            {
                UE_LOGFMT(LogTemp, Warning, "Time of Day Event = '{EventName}'",
                    UEnum::GetValueAsString(TimeOfDayEvent.Key));
                OnTimeOfDay.Broadcast(TimeOfDayEvent.Key);
            }
        }
    }
}

void AWfGameModeBase::DetermineSeason()
{
    const auto DayOfYear = GetGameDateTime().GetDayOfYear();
    for (const auto& SeasonDate : SeasonalDates)
    {
        if (DayOfYear >= SeasonDate.Key)
        {
            if (SeasonDate.Value != CurrentSeason)
            {
                SetCurrentSeason(SeasonDate.Value);
            }

            // Stop looping once we've met or passed the current day
            return;
        }
    }
}

void AWfGameModeBase::InitSeasonalTempAverages()
{
    SeasonalTempAverages.Add(EClimateSeason::EarlyWinter, 10.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidWinter, 8.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateWinter, 10.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlySpring, 15.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateSpring, 20.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlySummer, 25.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidSummer, 30.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateSummer, 28.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlyAutumn, 22.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidAutumn, 18.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateAutumn, 12.0f);
}

void AWfGameModeBase::InitDailyTemperatureRange()
{
    SeasonalTempAverages.Add(EClimateSeason::EarlyWinter, 8.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidWinter, 6.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateWinter, 8.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlySpring, 10.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateSpring, 12.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlySummer, 15.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidSummer, 18.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateSummer, 16.0f);
    SeasonalTempAverages.Add(EClimateSeason::EarlyAutumn, 10.0f);
    SeasonalTempAverages.Add(EClimateSeason::MidAutumn, 8.0f);
    SeasonalTempAverages.Add(EClimateSeason::LateAutumn, 6.0f);
}

void AWfGameModeBase::InitSeasonalDates()
{
    SeasonalDates.Add(FDateTime(2024, 1, 21).GetDayOfYear(), EClimateSeason::MidWinter);
    SeasonalDates.Add(FDateTime(2024, 2, 21).GetDayOfYear(), EClimateSeason::LateWinter);
    SeasonalDates.Add(FDateTime(2024, 3, 21).GetDayOfYear(), EClimateSeason::EarlySpring);
    SeasonalDates.Add(FDateTime(2024, 4, 21).GetDayOfYear(), EClimateSeason::LateSpring);
    SeasonalDates.Add(FDateTime(2024, 6, 21).GetDayOfYear(), EClimateSeason::EarlySummer);
    SeasonalDates.Add(FDateTime(2024, 7, 21).GetDayOfYear(), EClimateSeason::MidSummer);
    SeasonalDates.Add(FDateTime(2024, 8, 21).GetDayOfYear(), EClimateSeason::LateSummer);
    SeasonalDates.Add(FDateTime(2024, 9, 22).GetDayOfYear(), EClimateSeason::EarlyAutumn);
    SeasonalDates.Add(FDateTime(2024, 10, 22).GetDayOfYear(), EClimateSeason::MidAutumn);
    SeasonalDates.Add(FDateTime(2024, 11, 22).GetDayOfYear(), EClimateSeason::LateAutumn);
    SeasonalDates.Add(FDateTime(2024, 12, 21).GetDayOfYear(), EClimateSeason::EarlyWinter);
}

void AWfGameModeBase::DetermineTemperature()
{
    const float AvgTemp = SeasonalTempAverages.Contains(CurrentSeason)
                        ? SeasonalTempAverages[CurrentSeason] : 15.0f;

    const float TempRange = SeasonalTempRanges.Contains(CurrentSeason)
                            ? SeasonalTempRanges[CurrentSeason] : 10.0f;

    constexpr float MaxTempTime = 15.0f; // 3 PM is the peak time
    constexpr float MinTempTime = 5.0f; // 5 AM is the lowest temperature

    const float TimeOfDay = GetGameDateTime().GetHour();

    float Temp = AvgTemp;

    if (TimeOfDay < MaxTempTime)
    {
        // Morning ramp-up
        Temp += (TempRange / 2.0f) * FMath::Sin(((TimeOfDay - MinTempTime) / (MaxTempTime - MinTempTime)) * PI);
    }
    else
    {
        // Evening ramp-down
        Temp += (TempRange / 2.0f) * FMath::Sin(((TimeOfDay - MaxTempTime) / (24.0f - MaxTempTime + MinTempTime)) * PI);
    }

    TemperatureSurface = Temp;

    // Adjust temperature based on altitude difference and lapse rate
    constexpr float LapseRate = 6.5f; // Lapse rate (°C * km)
    const float AltitudeTempAdjustment = LapseRate * AltitudeDiff;
    const float AdjustedTemp = TemperatureSurface - AltitudeTempAdjustment;

    // Blend surface temperature and altitude temperature
    const float BlendFactor = FMath::Clamp(AltitudeDiff / 10.0f, 0.0f, 1.0f);
    TemperatureAltitude = FMath::Lerp(Temp, AdjustedTemp, BlendFactor);

    if (OnTemperatureChanged.IsBound())
        OnTemperatureChanged.Broadcast(TemperatureSurface, TemperatureAltitude);
}

void AWfGameModeBase::DetermineWindSpeed()
{
    //WeatherWindSpeed = 0.0f;
}

void AWfGameModeBase::DetermineRelativeHumidity()
{
    //WeatherHumidity = 0.0f;
}

void AWfGameModeBase::DetermineDroughtFactor()
{
    //DroughtFactor = 0.0f;
}

void AWfGameModeBase::DetermineWeatherCondition()
{
    //CurrentWeather = EWeatherCondition::Clear;
}

ETimeOfDay AWfGameModeBase::DetermineTimeOfDay()
{
    for (const auto& TimeOfDayEvent : TimeOfDayEvents)
    {
        if (GameDateTime.GetHour() >= TimeOfDayEvent.Value.GetTotalHours())
            continue;
        return TimeOfDayEvent.Key;
    }
    return ETimeOfDay::Noon;
}
