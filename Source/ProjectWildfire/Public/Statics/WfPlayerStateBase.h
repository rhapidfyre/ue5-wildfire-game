// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/WfFfCharacterBase.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerState.h"
#include "Saves/WfCharacterSaveGame.h"

#include "WfPlayerStateBase.generated.h"


class AAIController;
class AWfGameModeBase;
class AWfFireStationBase;
class UWfFirefighterSaveGame;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourceUpdated, const FGameplayTag&, ResourceTag, const float, OldValue, const float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFirefighterHired, const AWfFfCharacterBase*, ActorFirefighter);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFirefighterFired, const AWfFfCharacterBase*, ActorFirefighter);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFirefighterDeath, const AWfFfCharacterBase*, ActorFirefighter);

/**
 *	Includes all logic necessary for the player state to function in this custom game
 *	regardless of which type (campaign, testing, sandbox, etc.)
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:


	AWfPlayerStateBase();


	UFUNCTION(BlueprintCallable)
	AWfFfCharacterBase* AcceptJobContract(
	const FJobContractData& JobContract, FString& FailureContext);


	UFUNCTION(BlueprintPure)
	FGameplayTagContainer GetAllResourceTags() const;

	UFUNCTION(BlueprintPure)
	TArray<AWfFfCharacterBase*> GetAllPersonnel() const { return AllPersonnel; }

	UFUNCTION()
	AWfFireStationBase* GetFireStationReference();

	UFUNCTION(BlueprintPure)
	float GetMoney() const;

	UFUNCTION(BlueprintPure)
	float GetKilowattUsage() const;

	UFUNCTION(BlueprintPure)
	float GetOxygenReserve() const;

	UFUNCTION(BlueprintPure)
	float GetWaterStorage() const;

	UFUNCTION(BlueprintPure)
	float GetResourceValue(const FGameplayTag& ResourceTag) const;


	UFUNCTION()
	void SetFireStationReference(AWfFireStationBase* FireStation);


	UFUNCTION(BlueprintCallable)
	float AddMoney(float AddValue = 1.0f);

	UFUNCTION(BlueprintCallable)
	float AddKilowattUsage(float AddValue = 1.0f);

	UFUNCTION(BlueprintCallable)
	float AddOxygenReserve(float AddValue = 1.0f);

	UFUNCTION(BlueprintCallable)
	float AddWaterStorage(float AddValue = 1.0f);


	UFUNCTION(BlueprintCallable)
	float RemoveMoney(const float RemoveValue = 1.0f) { return AddMoney(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveKilowattUsage(const float RemoveValue = 1.0f) { return AddKilowattUsage(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveOxygenReserve(const float RemoveValue = 1.0f) { return AddOxygenReserve(-1 * RemoveValue); }

	UFUNCTION(BlueprintCallable)
	float RemoveWaterStorage(const float RemoveValue = 1.0f) { return AddWaterStorage(-1 * RemoveValue); }



	virtual void BeginPlay() override;

protected:

	UFUNCTION(Server, Reliable)
	void Server_AcceptContract(const FJobContractData& JobContract);

	UFUNCTION() void HourlyTick(const FDateTime& NewDateTime);

	virtual void SetupInitialResourceValues();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION(BlueprintCallable)
	void SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue = 0.0f);


	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FirefighterHired(const AWfFfCharacterBase* NewFirefighter);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FirefighterFired(const AWfFfCharacterBase* OldFirefighter);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FirefighterDeath(const AWfFfCharacterBase* DeadFirefighter);


	UFUNCTION(BlueprintCallable)
	void SetMoney(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetKilowattUsage(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetOxygenReserve(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetWaterStorage(float NewValue = 0.0f);

public:

	UPROPERTY(BlueprintAssignable)
	FOnResourceUpdated OnResourceUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnFirefighterHired OnFirefighterHired;

	UPROPERTY(BlueprintAssignable)
	FOnFirefighterFired OnFirefighterFired;

	UPROPERTY(BlueprintAssignable)
	FOnFirefighterDeath OnFirefighterDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	TSubclassOf<AAIController> UsingAiController;

protected:

	UPROPERTY(Replicated)
	TArray<AWfFfCharacterBase*> AllPersonnel;

private:

	UPROPERTY()
	AWfFireStationBase* FireStationBase;

	UPROPERTY()
	AWfGameModeBase* GameModeBase;

	TMap<FGameplayTag, float> Resources;

};
