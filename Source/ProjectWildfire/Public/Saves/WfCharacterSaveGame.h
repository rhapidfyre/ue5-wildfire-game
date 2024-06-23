// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WfSaveGame.h"

#include "WfCharacterSaveGame.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API UWfCharacterSaveGame : public UWfSaveGame
{
	GENERATED_BODY()

public:

	UWfCharacterSaveGame(): UWfSaveGame() {};

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

	UWfFirefighterSaveGame(): YearsOfService(0), YearsInGrade(0), HourlyRate(0)	{}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsOfService;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	int YearsInGrade;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Firefighter Save Data")
	float HourlyRate;
};
