// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfGameModeBase.h"

#include "Actors/GameManager.h"
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
    : FirstNamesTable(nullptr),
      LastNamesTable(nullptr),
      MessageTable(nullptr),
      VoxTable(nullptr),
      CalloutsTable(nullptr),
      bUseSeasons(false),
      bUseMetricSystem(false)
{
}

void AWfGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
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

    for (int32 i = 1; i < NumbersTaken.Num() + 1; ++i)
    {
        if (!NumbersTaken.Contains(i))
        {
            return i;
        }
    }
    return -1;
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

    AGameManager::GetInstance(GetWorld()); // Create the Game Manager, if it doesn't exist already

    GenerateJobContracts();
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

void AWfGameModeBase::GenerateJobContracts()
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

    // Remove expired hiring offers
    for (auto& FirefighterOffer : FirefightersUnemployed)
    {
        if (FirefighterOffer->OfferExpiration < FDateTime::UtcNow())
        {
            JobContractExpired(FJobContractData(FirefighterOffer));
        }
    }
}

void AWfGameModeBase::JobContractExpired(const FJobContractData& JobContract, bool bDeleteSave)
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
                    if (bDeleteSave)
                    {
                        UGameplayStatics::DeleteGameInSlot(JobContract.ContractId, JobContract.UserIndex);
                    }
                    return;
                }
            }
        }
    }
}
