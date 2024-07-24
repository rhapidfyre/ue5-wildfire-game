// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfGameModeBase.h"

#include "Actors/WfFireStationBase.h"
#include "Characters/WfCharacterTags.h"
#include "Characters/WfCharacterData.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


FString GenerateRandomString(int32 Length)
{
    const TCHAR Charset[] = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    const int32 MaxIndex = UE_ARRAY_COUNT(Charset) - 2; // -2 to account for null terminator

    FString RandomString;
    RandomString.Reserve(Length);

    for (int32 i = 0; i < Length; ++i)
    {
        int32 Index = FMath::RandRange(0, MaxIndex);
        RandomString.AppendChar(Charset[Index]);
    }

    return RandomString;
}


AWfGameModeBase::AWfGameModeBase()
    : FirstNamesTable(nullptr)
    ,  LastNamesTable(nullptr)
    ,  MessageTable(nullptr)
    ,  bUseSeasons(false)
    ,  bUseMetricSystem(false)
    ,  TemperatureSurface(24.0f)
    ,  TemperatureAltitude(24.0f)
    ,  AltitudeDiff(1)
    ,  WeatherAirStability(0)
    ,  WeatherWindSpeed(0)
    ,  WeatherHumidity(0)
    ,  DroughtFactor(0)
    ,  CurrentSeason(EClimateSeason::EarlySpring)
    ,  CurrentWeather(EWeatherCondition::Clear)
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

int AWfGameModeBase::GetNextFireStationNumber() const
{
    TArray<AActor*> OutActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWfFireStationBase::StaticClass(), OutActors);

    TSet<int32> NumbersTaken;
    for (const auto& StationActor : OutActors)
    {
        if (AWfFireStationBase* FireStation = Cast<AWfFireStationBase>(StationActor))
        {
            NumbersTaken.Add(FireStation->FireStationNumber);
        }
    }

    for (int32 i = 1; ; ++i)
    {
        if (!NumbersTaken.Contains(i))
        {
            return i;
        }
    }
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

TArray<FString> AWfGameModeBase::GenerateRandomName(const FGameplayTag& Gender, const FGameplayTag& Ethnicity) const
{
    // Ensure data tables are valid
    FString NameFirst, NameMiddle, NameLast;
    if (!FirstNamesTable || !LastNamesTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Name DataTables are not set!"));
        NameFirst	= Gender == TAG_Gender_Male.GetTag() ? "John" : "Jane";
        NameMiddle	= "Q";
        NameLast	= "Public";
        return {NameFirst, NameMiddle, NameLast};
    }

    // Get random first name
    const FString FirstNameContext = "";
    TArray<FWfNamesStruct*> FirstNameRows;
    FirstNamesTable->GetAllRows<FWfNamesStruct>(FirstNameContext, FirstNameRows);
    TArray<FWfNamesStruct*> FirstNames;
    for (const auto& FirstNameRow : FirstNameRows)
    {
        if (FirstNameRow->EthnicGroups.HasTag(Ethnicity))
        {
            if (FirstNameRow->bFeminine && Gender != TAG_Gender_Male.GetTag())
            { FirstNames.Add(FirstNameRow); }
            else if (FirstNameRow->bMasculine && Gender != TAG_Gender_Female.GetTag())
            { FirstNames.Add(FirstNameRow); }
        }
    }
    int32 FirstNameIndex = FMath::RandRange(0, FirstNames.Num() - 1);
    NameFirst = FirstNames[FirstNameIndex]->NameValue;

    // Get random last name
    const FString LastNameContext;
    TArray<FWfNamesStruct*> LastNameRows;
    LastNamesTable->GetAllRows<FWfNamesStruct>(LastNameContext, LastNameRows);
    TArray<FWfNamesStruct*> LastNames;
    for (const auto& LastNameRow : LastNameRows)
    {
        if (LastNameRow->EthnicGroups.HasTag(Ethnicity))
        {
            if (LastNameRow->bFeminine && Gender != TAG_Gender_Male.GetTag())
            { LastNames.Add(LastNameRow); }
            else if (LastNameRow->bMasculine && Gender != TAG_Gender_Female.GetTag())
            { LastNames.Add(LastNameRow); }
        }
    }
    int32 LastNameIndex = FMath::RandRange(0, LastNames.Num() - 1);
    NameLast = LastNames[LastNameIndex]->NameValue;

    // Get random middle initial
    char MiddleInitialChar = static_cast<char>(FMath::RandRange(65, 90));
    NameMiddle = FString(1, &MiddleInitialChar);
    return {NameFirst, NameMiddle, NameLast};
}

FGameplayTag AWfGameModeBase::PickRandomEthnicGroup()
{

    TArray<FEthnicGroup> EthnicGroups = {
        {TAG_Ethnicity_White.GetTag(), 0.616},
        {TAG_Ethnicity_Black.GetTag(), 0.124},
        {TAG_Ethnicity_Hispanic.GetTag(), 0.187},
        {TAG_Ethnicity_Asian.GetTag(), 0.06},
        {TAG_Ethnicity_NativeAmerican.GetTag(), 0.029},
        {TAG_Ethnicity_PacificIslander.GetTag(), 0.002}
    };

    double RandValue = FMath::FRand();
    double CumulativeChance = 0.0;

    for (const auto& [GroupName, Chance] : EthnicGroups)
    {
        CumulativeChance += Chance;
        if (RandValue < CumulativeChance)
        {
            return GroupName;
        }
    }

    return EthnicGroups[0].GroupName;
}

FGameplayTag AWfGameModeBase::PickRandomFatherEthnicGroup(const FGameplayTag& MotherEthnicGroup)
{
    double InterracialChance = 0.151;
    double RandValue = FMath::FRand();

    if (RandValue < InterracialChance)
        return PickRandomEthnicGroup();
    return MotherEthnicGroup;
}

FGameplayTag AWfGameModeBase::DetermineMixedRaceOutcome(
    const FGameplayTag& MotherEthnicGroup, const FGameplayTag& FatherEthnicGroup)
{
    if (MotherEthnicGroup != FatherEthnicGroup)
    {
        return FMath::RandRange(0,1) == 0 ? MotherEthnicGroup : FatherEthnicGroup;
    }
    return FGameplayTag(MotherEthnicGroup);
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

FJobContractData AWfGameModeBase::ConvertSaveToJobContract(const UWfFirefighterSaveGame* SaveGame)
{
    if (IsValid(SaveGame))
    {
        FJobContractData JobContract(SaveGame);
        return JobContract;
    }
    return {};
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

    if (OnGameHourlyTick.IsBound())
    {
        if (OldDateTime.GetHour() < NewDateTime.GetHour())
        {
            OnGameHourlyTick.Broadcast(NewDateTime);
        }
    }

    // Generating saves in BeginPlay causes crashes
    if (bFirstRun)
    {
        for (int i = FirefightersUnemployed.Num(); i < 10; ++i)
        {
            UWfSaveGame* NewSaveGame = CreateNewCharacter(TAG_Role_Fire.GetTag());
            UWfFirefighterSaveGame* FirefighterSaveGame = Cast<UWfFirefighterSaveGame>(NewSaveGame);
            if (IsValid(FirefighterSaveGame))
            {
                JobContractOffer(FirefighterSaveGame);
            }
        }
        bFirstRun = false;
    }

    // Remove expired hiring offers
    for (auto& FirefighterOffer : FirefightersUnemployed)
    {
        if (FirefighterOffer->OfferExpiration < FDateTime::UtcNow())
        {
            JobContractExpired(FJobContractData(FirefighterOffer));
        }
    }

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

                // Every day at Start of Business refresh the roster back up to 10
                if (TimeOfDayEvent.Key == ETimeOfDay::StartOfBusiness)
                {
                    for (int i = FirefightersUnemployed.Num(); i < 10; ++i)
                    {
                        UWfSaveGame* NewSaveGame = CreateNewCharacter(TAG_Role_Fire.GetTag());
                        UWfFirefighterSaveGame* FirefighterSaveGame = Cast<UWfFirefighterSaveGame>(NewSaveGame);
                        if (IsValid(FirefighterSaveGame))
                            JobContractOffer(FirefighterSaveGame);
                    }
                }
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

FGameplayTag AWfGameModeBase::GenerateRandomGender(const FGameplayTag& CharacterRole)
{
    // If the character isn't a firefighter, use a 50-50 chance
    float GenderRate = CharacterRole.MatchesTag(TAG_Role_Fire.GetTag())
                     ? 0.31 : 0.50;

    const float GenderChance = FMath::RandRange(0.0f, 1.0f);
    if (GenderChance < GenderRate)
    {
        if (GenderChance < 0.01)
            return TAG_Gender_Non_Binary.GetTag();
        return TAG_Gender_Female.GetTag();
    }
    return TAG_Gender_Male.GetTag();
}

FGameplayTag AWfGameModeBase::GenerateRandomRole(const FGameplayTag& PrimaryRole)
{
    // Determine sub-role, if primary role tag is a fire tag
    if (PrimaryRole.MatchesTagExact(TAG_Role_Fire.GetTag()))
    {
        const float RandomValue = FMath::FRand(); // Random value between 0 and 1

        // Define the probabilities for each role
        constexpr float ChiefProbability	= 0.02f;
        constexpr float CptProbability		= 0.03f;
        constexpr float EngProbability		= 0.12f;
        constexpr float SrProbability		= 0.20f;

        // Determine the role based on the random value
        if (RandomValue < ChiefProbability)
            return TAG_Role_Fire_Chief.GetTag();
        if (RandomValue < ChiefProbability + CptProbability)
            return TAG_Role_Fire_Cpt.GetTag();
        if (RandomValue < ChiefProbability + CptProbability + EngProbability)
            return TAG_Role_Fire_Eng.GetTag();
        if (RandomValue < ChiefProbability + CptProbability + EngProbability + SrProbability)
            return TAG_Role_Fire_Two.GetTag();
        return TAG_Role_Fire_One.GetTag();
    }
    // Otherwise, use the role tag as given
    return PrimaryRole;
}

FGameplayTag AWfGameModeBase::GenerateRandomRace()
{
    // Generate mother's race
    const FGameplayTag MotherRace = PickRandomEthnicGroup();

    // Generate father's race
    const FGameplayTag FatherRace = PickRandomFatherEthnicGroup(MotherRace);

    // Determine mixed race outcome
    return DetermineMixedRaceOutcome(MotherRace, FatherRace);
}

int AWfGameModeBase::GenerateRandomAge(const FGameplayTag& CharacterRole)
{
    const float RandomValue = FMath::FRand();

    int MinAge = 18;
    int MaxAge = 99;
    TArray<float> AgeProbabilities;
    TArray<int>   AgeRanges;

    // If the character isn't a firefighter, use a basic random age
    if (!CharacterRole.MatchesTag(TAG_Role_Fire.GetTag()))
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.25)
            return FMath::RandRange(36, 99);
        return FMath::RandRange(18, 65);
    }

    if (CharacterRole == TAG_Role_Fire_Chief.GetTag())
    {
        // Example ranges: [35-49, 50-64, 65-70]
        AgeRanges = {35, 50, 65, 70};

        // Probabilities corresponding to age ranges
        AgeProbabilities = {0.1f, 0.7f, 0.2f};
    }
    else if (CharacterRole == TAG_Role_Fire_Cpt.GetTag())
    {
        AgeRanges = {30, 40, 55, 60};
        AgeProbabilities = {0.2f, 0.5f, 0.3f};
    }
    else if (CharacterRole == TAG_Role_Fire_Eng.GetTag())
    {
        AgeRanges = {20, 30, 40, 50};
        AgeProbabilities = {0.4f, 0.4f, 0.2f};
    }
    else if (CharacterRole == TAG_Role_Fire_Two.GetTag())
    {
        AgeRanges = {18, 25, 35, 45};
        AgeProbabilities = {0.5f, 0.3f, 0.2f};
    }
    else if (CharacterRole == TAG_Role_Fire_One.GetTag())
    {
        AgeRanges = {18, 22, 28, 35};
        AgeProbabilities = {0.6f, 0.3f, 0.1f};
    }

    // Determine the age range based on probabilities
    float CumulativeProbability = 0.0f;
    for (int i = 0; i < AgeProbabilities.Num(); ++i)
    {
        CumulativeProbability += AgeProbabilities[i];
        if (RandomValue < CumulativeProbability)
        {
            MinAge = AgeRanges[i];
            MaxAge = AgeRanges[i + 1] - 1;
            break;
        }
    }

    return FMath::RandRange(MinAge, MaxAge);
}

float AWfGameModeBase::CalculateHourlyRate(const FGameplayTag& CharacterRole, int YearsOfService)
{
    YearsOfService = FMath::Clamp(YearsOfService, 0, 25);
    float HourlyPay = 3.0f;
    if (CharacterRole == TAG_Role_Fire_One.GetTag())
        HourlyPay = 21.0f;
    else if (CharacterRole == TAG_Role_Fire_Two.GetTag())
        HourlyPay = 23.0f;
    else if (CharacterRole == TAG_Role_Fire_Eng.GetTag())
        HourlyPay = 25.0f;
    else if (CharacterRole == TAG_Role_Fire_Cpt.GetTag())
        HourlyPay = 31.0f;
    else if (CharacterRole == TAG_Role_Fire_Chief.GetTag())
        HourlyPay = 43.0f;
    else if (CharacterRole == TAG_Role_Fire_Div.GetTag())
        HourlyPay = 52.0f;
    else if (CharacterRole == TAG_Role_Fire_Pilot.GetTag())
        HourlyPay = 32.0f;
    else if (CharacterRole == TAG_Role_Fire_Arson.GetTag())
        HourlyPay = 28.0f;
    else if (CharacterRole == TAG_Role_Fire_New.GetTag())
        HourlyPay = 20.0f;
    for (int i = 1; i <= YearsOfService; ++i)
    {
        HourlyPay *= 1.032f;
    }
    return HourlyPay;
}

/**
 * \brief Generates a brand-new character
 * \param NewCharacterRole The highest level role to assume.
 *                          If given a primary role, a secondary/sub-role will be generated.
 */
UWfSaveGame* AWfGameModeBase::CreateNewCharacter(const FGameplayTag& NewCharacterRole)
{
    USaveGame* SaveGame = nullptr;
    if (NewCharacterRole.MatchesTag(TAG_Role_Fire.GetTag()))
        SaveGame = UGameplayStatics::CreateSaveGameObject( UWfFirefighterSaveGame::StaticClass() );
    else if (NewCharacterRole.MatchesTag(TAG_Role_Police.GetTag()))
        SaveGame = UGameplayStatics::CreateSaveGameObject( UWfSaveGame::StaticClass() /* UWfOfficerSaveGame::StaticClass() */ );
    else if (NewCharacterRole.MatchesTag(TAG_Role_Civilian.GetTag()))
        SaveGame = UGameplayStatics::CreateSaveGameObject( UWfSaveGame::StaticClass() /* UWfCivilianSaveGame::StaticClass() */ );
    else if (NewCharacterRole.MatchesTag(TAG_Role_Player.GetTag()))
        SaveGame = UGameplayStatics::CreateSaveGameObject( UWfSaveGame::StaticClass() /* UWfPlayerSaveGame::StaticClass() */);

    if (!IsValid(SaveGame))
        return nullptr;

    // Initialization that applies to all save games
    UWfSaveGame* GameSave = Cast<UWfSaveGame>(SaveGame);
    if (!IsValid(GameSave))
        return nullptr;

    // Initialization that applies to all characters
    UWfCharacterSaveGame* NewCharacter = Cast<UWfCharacterSaveGame>(SaveGame);
    if (IsValid(NewCharacter))
    {
        NewCharacter->CharacterRole   = GenerateRandomRole(NewCharacterRole);
        NewCharacter->CharacterAge    = GenerateRandomAge(NewCharacter->CharacterRole);
        NewCharacter->CharacterGender = GenerateRandomGender(NewCharacter->CharacterRole);
        const FGameplayTag MomEthnicity = PickRandomEthnicGroup();
        const FGameplayTag DadEthnicity = PickRandomFatherEthnicGroup(MomEthnicity);
        NewCharacter->CharacterRace   = DetermineMixedRaceOutcome(MomEthnicity, DadEthnicity);

        TArray<FString> CharacterName = GenerateRandomName(NewCharacter->CharacterGender, NewCharacter->CharacterRace);
        NewCharacter->CharacterNameFirst  = CharacterName[0];
        NewCharacter->CharacterNameMiddle = CharacterName[1];
        NewCharacter->CharacterNameLast   = CharacterName[2];
    }

    // Initialization that applies to firefighters
    UWfFirefighterSaveGame* NewFirefighter = Cast<UWfFirefighterSaveGame>(SaveGame);
    if (IsValid(NewFirefighter))
    {
        NewFirefighter->HourlyRate      = CalculateHourlyRate(NewFirefighter->CharacterRole, 0);
        NewFirefighter->OfferExpiration = FDateTime::UtcNow()
            + FTimespan(FMath::RandRange(0,5), FMath::RandRange(4, 23), 0, 0);
    }

    UGameplayStatics::SaveGameToSlot(GameSave, GameSave->SaveSlotName, GameSave->SaveSlotIndex);
    return GameSave;
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

void AWfGameModeBase::JobContractOffer(USaveGame* SaveGame)
{
    UWfFirefighterSaveGame* FfSaveGame = Cast<UWfFirefighterSaveGame>(SaveGame);
    if (IsValid(FfSaveGame))
    {
        FRWScopeLock WriteLock(TransferListRWLock, SLT_Write);
        FirefightersUnemployed.Add(FfSaveGame);
        const FJobContractData JobContract(FfSaveGame);
        if (OnJobContractOffer.IsBound())
            OnJobContractOffer.Broadcast(JobContract);
    }
}

void AWfGameModeBase::JobContractExpired(const FJobContractData& JobContract)
{
    const UWfFirefighterSaveGame* SaveGame = JobContract.SaveReference;
    if (IsValid(SaveGame))
    {
        FRWScopeLock WriteLock(TransferListRWLock, SLT_Write);
        for (int i = 0; i < FirefightersUnemployed.Num(); ++i)
        {
            if (FirefightersUnemployed.IsValidIndex(i))
            {
                UWfFirefighterSaveGame* FirefighterSave = FirefightersUnemployed[i];
                if (FirefighterSave == SaveGame)
                {
                    UE_LOGFMT(LogTemp, Display, "Job Contract #{ContractId} for '{FfName}' expired or was deleted."
                        , FirefighterSave->SaveSlotName, FirefighterSave->CharacterNameFirst + FirefighterSave->CharacterNameLast);
                    FirefightersUnemployed.RemoveAt(i);
                    if (OnJobContractExpired.IsBound())
                    {
                        OnJobContractExpired.Broadcast(JobContract);
                    }
                    UGameplayStatics::DeleteGameInSlot(JobContract.ContractId, JobContract.UserIndex);
                    return;
                }
            }
        }
    }
}
