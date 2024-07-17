// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Character.h"
#include "Interfaces/WfClickableInterface.h"

#include "WfCharacterBase.generated.h"

class UWfAttributeSet;
class UWfAbilityComponent;
class USaveGame;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterGenderSet, const FGameplayTag&, NewGender);
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

	UFUNCTION(BlueprintCallable) void Save() { SaveCharacter(); };

	virtual void SaveCharacter();

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
	FGameplayTag GetCharacterRace() const { return EthnicGroup; }

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	void SetCharacterRole(const FGameplayTag& NewRole);

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	void SetCharacterGender(const FGameplayTag& NewGender);

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	void SetCharacterAge(const int NewAge);

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	void SetCharacterName(const TArray<FString>& NewCharacterName);

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	void SetCharacterRace(const FGameplayTag& NewCharacterRace);

	virtual void LoadCharacter(USaveGame* CharacterSaveGame);

	virtual void NewCharacter(const FGameplayTag& NewPrimaryRole);


protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(NetMulticast, Reliable) void OnRep_NameFirst(const FString& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_NameMiddle(const FString& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_NameLast(const FString& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_CharacterAge(const int8& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_EthnicGroup(const FGameplayTag& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_CharacterGender(const FGameplayTag& OldValue);
	UFUNCTION(NetMulticast, Reliable) void OnRep_CharacterRole(const FGameplayTag& OldValue);

public:

	UPROPERTY(BlueprintAssignable) FOnCharacterNameSet   OnCharacterNameSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterAgeSet	 OnCharacterAgeSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterGenderSet OnCharacterGenderSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterRaceSet	 OnCharacterRaceSet;
	UPROPERTY(BlueprintAssignable) FOnCharacterRoleSet	 OnCharacterRoleSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Data")
	UWfAbilityComponent* AbilityComponent;

	// All possible roles this character can take when spawning
	// If empty, it will pick a random, most-appropriate role for the lowest child subclass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	FGameplayTagContainer PossibleRoles;

private:

	UPROPERTY(ReplicatedUsing = OnRep_NameFirst)		FString NameFirst;
	UPROPERTY(ReplicatedUsing = OnRep_NameMiddle)		FString NameMiddle;
	UPROPERTY(ReplicatedUsing = OnRep_nameLast)			FString NameLast;
	UPROPERTY(ReplicatedUsing = OnRep_CharacterAge)		int8	CharacterAge;
	UPROPERTY(ReplicatedUsing = OnRep_EthnicGroup)		FGameplayTag EthnicGroup;
	UPROPERTY(ReplicatedUsing = OnRep_CharacterGender)	FGameplayTag CharacterGender;
	UPROPERTY(ReplicatedUsing = OnRep_CharacterRole)	FGameplayTag CharacterRole;

	UPROPERTY() UWfAttributeSet* CharacterAttributes;

};
