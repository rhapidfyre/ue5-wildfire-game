// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Character.h"
#include "Interfaces/WfClickableInterface.h"

#include "WfCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterGenderSet,	const FGameplayTag&, NewGender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterAgeSet,		const int8,			 NewAge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterRaceSet,	const FGameplayTag&, NewRace);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterRoleSet,	const FGameplayTag&, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterNameSet,	const FString&,		 NewName);


UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API AWfCharacterBase : public ACharacter, public IWfClickableInterface
{
	GENERATED_BODY()

public:

	AWfCharacterBase();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual bool EventPrimarySelect() override;

	virtual bool EventSecondarySelect() override;

	UFUNCTION(BlueprintPure, Category = "Character Data")
	TArray<FString> GetCharacterNames() const;

	UFUNCTION(BlueprintPure, Category = "Character Data")
	FString GetCharacterName() const;

	UFUNCTION(BlueprintPure, Category = "Character Data")
	int GetCharacterAge() const { return CharacterAge; }

	UFUNCTION(BlueprintPure, Category = "Character Data")
	FGameplayTag GetCharacterGender() const { return CharacterGender; }

	UFUNCTION(BlueprintPure, Category = "Character Data")
	FGameplayTag GetCharacterRole() const { return CharacterRole; }

	UFUNCTION(BlueprintPure, Category = "Character Data")
	FGameplayTag GetEthnicity() const { return EthnicGroup; }

protected:

	virtual void BeginPlay() override;

private:

	FGameplayTag PickRandomEthnicGroup();

	FGameplayTag PickRandomFatherEthnicGroup(const FGameplayTag& MotherEthnicGroup);

	void DetermineMixedRaceOutcome(
		const FGameplayTag& MotherEthnicGroup, const FGameplayTag& FatherEthnicGroup);

	void GenerateRandomName();

	void GenerateRandomRace();

public:

	UPROPERTY(BlueprintAssignable) FOnCharacterNameSet   OnCharacterNameSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterAgeSet	 OnCharacterAgeSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterGenderSet OnCharacterGenderSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterRaceSet	 OnCharacterRaceSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterRoleSet	 OnCharacterRoleSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	UDataTable* FirstNamesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	UDataTable* LastNamesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	FGameplayTag CharacterRole;

private:

	FString NameFirst;
	FString NameMiddle;
	FString NameLast;

	int8 CharacterAge;

	FGameplayTag EthnicGroup;
	FGameplayTag CharacterGender;

};
