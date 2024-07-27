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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnTransferListUpdated, const FJobContractData&, ContractId, const bool, bIsAvailable);


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

	UFUNCTION(BlueprintCallable) USaveGame* CreateNewCharacter(const FGameplayTag& GetRole);

	UFUNCTION(BlueprintPure)
	TArray<UWfFirefighterSaveGame*> GetTransferList() const;

	UFUNCTION(BlueprintCallable)
	UWfFirefighterSaveGame* GetSaveGameFromJobContract(const FJobContractData& JobContract);

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(Client, Reliable)
	void OnRep_HiringList(const TArray<FJobContractData>& OldHiringList);

public:

	UPROPERTY(BlueprintAssignable) FOnTransferListUpdated   OnTransferListUpdated;

private:

	// A set interval to update the client's game time awareness
	// Defaults to once per real-world second (1.0f)
	FTimerHandle GameDateTimeChecker;
	UPROPERTY()	AWfGameModeBase* GameModeBase;

	UPROPERTY(ReplicatedUsing = OnRep_HiringList)
	TArray<FJobContractData> HiringList;
};
