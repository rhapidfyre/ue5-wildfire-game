// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfGlobalEnums.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameModeBase.h"
#include "Saves/WfCharacterSaveGame.h"

#include "WfGameModeBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJobContractOffer, const FJobContractData&, JobContractData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJobContractExpired, const FJobContractData&, JobContractData);


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

	UDataTable* GetMessageTable() const { return MessageTable; }

	int GetNextFireStationNumber() const;

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	TArray<FString> GenerateRandomName(
			const FGameplayTag& Gender, const FGameplayTag& Ethnicity) const;

	UFUNCTION(BlueprintPure, Category = "Game Data")
	TArray<UWfFirefighterSaveGame*> GetListOfTransfers() const { return FirefightersUnemployed; };

	static FGameplayTag PickRandomEthnicGroup();
	static FGameplayTag PickRandomFatherEthnicGroup(const FGameplayTag& MotherEthnicGroup);
	static FGameplayTag DetermineMixedRaceOutcome(const FGameplayTag& MotherEthnicGroup, const FGameplayTag& FatherEthnicGroup);
	static FGameplayTag GenerateRandomGender(const FGameplayTag& CharacterRole);
	static FGameplayTag GenerateRandomRole(const FGameplayTag& PrimaryRole);
	static FGameplayTag GenerateRandomRace();
	static int			GenerateRandomAge(const FGameplayTag& CharacterRole);
	static float CalculateHourlyRate(const FGameplayTag& CharacterRole, int YearsOfService);

	UWfSaveGame* CreateNewCharacter(const FGameplayTag& NewCharacterRole);

	virtual void JobContractExpired(const FJobContractData& JobContract, bool bDeleteSave = true);

protected:

	UFUNCTION(BlueprintCallable)
	static FJobContractData ConvertSaveToJobContract(const UWfFirefighterSaveGame* SaveGame);

	virtual void BeginPlay() override;

	virtual void JobContractOffer(USaveGame* SaveGame);

private:

	void GenerateJobContracts();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	UDataTable* FirstNamesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	UDataTable* LastNamesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
	UDataTable* MessageTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
	UDataTable* VoxTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
	UDataTable* CalloutsTable;

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

	UPROPERTY(BlueprintAssignable)  FOnJobContractOffer OnJobContractOffer;
	UPROPERTY(BlueprintAssignable)  FOnJobContractExpired OnJobContractExpired;

private:



	// List of firefighters that can be hired
	bool bFirstRun = true;
	UPROPERTY() TArray<UWfFirefighterSaveGame*> FirefightersUnemployed;

	mutable FRWLock TransferListRWLock;
};
