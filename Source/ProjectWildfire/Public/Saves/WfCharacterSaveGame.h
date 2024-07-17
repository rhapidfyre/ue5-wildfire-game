// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WfSaveGame.h"
#include "Statics/WfUtilities.h"

#include "WfCharacterSaveGame.generated.h"

class AWfFfCharacterBase;

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API UWfCharacterSaveGame : public UWfSaveGame
{
	GENERATED_BODY()

public:

	UWfCharacterSaveGame(): CharacterAge(0)
	{
		SaveSlotIndex = 0;
		if (SaveSlotName == "Untitled")
		{
			do
			{
				SaveSlotName = UWfUtilities::GenerateGUID();
				UE_LOGFMT(LogTemp, Display, "Character Save Generated: '{NewGuid}' (User Index #{uIndex})"
					, SaveSlotName, SaveSlotIndex);
			} while (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveSlotIndex));
		}
	} ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameFirst;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameMiddle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameLast;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterGender;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	int CharacterAge;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterRole;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterRace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FDateTime OfferExpiration;

};

UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API UWfFirefighterSaveGame : public UWfCharacterSaveGame
{
	GENERATED_BODY()

public:

	UWfFirefighterSaveGame(): YearsOfService(0), YearsInGrade(0), HourlyRate(0)
	{
		SaveSlotIndex = 0;
		if (SaveSlotName == "Untitled")
		{
			do
			{
				SaveSlotName = UWfUtilities::GenerateGUID();
				UE_LOGFMT(LogTemp, Display, "Firefighter Save Generated: '{NewGuid}' (User Index #{uIndex})"
					, SaveSlotName, SaveSlotIndex);
			} while (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveSlotIndex));
		}
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	TSubclassOf<AWfFfCharacterBase> FirefighterClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsOfService;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsInGrade;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	float HourlyRate;
};


USTRUCT(BlueprintType, Blueprintable)
struct PROJECTWILDFIRE_API FJobContractData
{
	GENERATED_BODY()

	FJobContractData()
		: SaveReference(nullptr), ContractId("None")
	    , CharacterAge(0), YearsOfService(0), YearsInGrade(0), HourlyRate(0)
	{
		UE_LOGFMT(LogTemp, Display, "JobContractOffer(): New Default Job Contract");
	}

	explicit FJobContractData(const UWfFirefighterSaveGame* SaveGamePtr)
		: SaveReference(SaveGamePtr), ContractId("None")
	    , CharacterAge(0), YearsOfService(0), YearsInGrade(0), HourlyRate(0)
	{
		if (IsValid(SaveReference))
		{
			ContractId				= SaveReference->SaveSlotName;
			UserIndex				= SaveReference->SaveSlotIndex;
			YearsOfService			= SaveReference->YearsOfService;
			YearsInGrade			= SaveReference->YearsInGrade;
			HourlyRate				= SaveReference->HourlyRate;
			CharacterAge			= SaveReference->CharacterAge;
			CharacterNameFirst		= SaveReference->CharacterNameFirst;
			CharacterNameMiddle		= SaveReference->CharacterNameMiddle;
			CharacterNameLast		= SaveReference->CharacterNameLast;
			CharacterGender 		= SaveReference->CharacterGender;
			CharacterRole 			= SaveReference->CharacterRole;
			CharacterRace 			= SaveReference->CharacterRace;
			OfferExpiration 		= SaveReference->OfferExpiration;

			UE_LOGFMT(LogTemp, Display
				, "JobContractOffer(): New Job Contract Created: '{FirstName} {MiddleName} {LastName}' ({ContractGuid})"
				, CharacterNameFirst
				, CharacterNameMiddle[0] + "."
				, CharacterNameLast
				, ContractId);
		}
	}

	UPROPERTY() const UWfFirefighterSaveGame* SaveReference;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString ContractId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	int32 UserIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameFirst;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameMiddle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FString CharacterNameLast;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterGender;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	int CharacterAge;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterRole;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FGameplayTag CharacterRace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Save Data")
	FDateTime OfferExpiration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsOfService;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsInGrade;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	float HourlyRate;

	// Equality operator
	bool operator==(const FJobContractData& Other) const
	{
		return ContractId == Other.ContractId;
	}

	// Inequality operator
	bool operator!=(const FJobContractData& Other) const
	{
		return !(*this == Other);
	}
};
