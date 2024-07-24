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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFireApparatusPurchaseFail, const FText&, FailureReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFireApparatusPurchase, const AWfFireApparatusBase*, NewFireApparatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFireApparatusDestroyed, const AWfFireApparatusBase*, NewFireApparatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFireStationChanged, const AWfFireStationBase*, NewFireStation, const bool, bIsOwned);


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FFirefighterAssignment
{
	GENERATED_BODY()

	FFirefighterAssignment();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter Assignment")
	AWfFireApparatusBase* AssignedVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter Assignment")
	AWfFfCharacterBase* CharacterReference;

};


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FFireApparatusFleet
{
	GENERATED_BODY()

	FFireApparatusFleet();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	AWfFireApparatusBase* VehicleReference;

	// Which fire station the apparatus is assigned to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fleet")
	AWfFireStationBase* FireStationBase;

};


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FFleetPurchaseData
{
	GENERATED_BODY()

	FFleetPurchaseData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	TSubclassOf<AWfFireApparatusBase> FireApparatusType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	float PurchaseValue;

};


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
	TArray<FFirefighterAssignment> GetAllPersonnel() const { return PersonnelData; }

	UFUNCTION(BlueprintPure)
	TArray<FFireApparatusFleet> GetVehicleFleetData() const { return FleetData; }

	UFUNCTION(BlueprintPure)
	AWfFireStationBase* GetFireStationReference() const;

	UFUNCTION(BlueprintCallable)
	void PurchaseFireApparatus(const FFleetPurchaseData& PurchaseData);

	UFUNCTION(BlueprintCallable)
	void SetAssignedApparatus(AWfFfCharacterBase* FireCharacter, AWfFireApparatusBase* FireApparatus);

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


	UFUNCTION(BlueprintCallable)
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

	UFUNCTION(Server, Reliable)
	void Server_PurchaseFireApparatus(const FFleetPurchaseData& PurchaseData);

	UFUNCTION() void HourlyTick(const FDateTime& NewDateTime);

	virtual void SetupInitialResourceValues();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_PurchaseError(const FText& ErrorReason);


	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetFireStation(AWfFireStationBase* FireStation);

private:

	UFUNCTION(BlueprintCallable)
	void SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue = 0.0f);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_FireStation();

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

	UPROPERTY(BlueprintAssignable)
	FOnFireStationChanged OnFireStationChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFireApparatusPurchase OnFireApparatusPurchase;

	UPROPERTY(BlueprintAssignable)
	FOnFireApparatusDestroyed OnFireApparatusDestroyed;

	UPROPERTY(BlueprintAssignable)
	FOnFireApparatusPurchaseFail OnFireApparatusPurchaseFail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	TSubclassOf<AAIController> UsingAiController;

protected:

	// All hired firefighters belonging to the player
	UPROPERTY(Replicated) TArray<FFirefighterAssignment> PersonnelData;

	// All fire apparatus owned by the player
	UPROPERTY(Replicated) TArray<FFireApparatusFleet>	 FleetData;

private:

	UPROPERTY(ReplicatedUsing=OnRep_FireStation)
	AWfFireStationBase* FireStationBase;

	UPROPERTY()
	AWfGameModeBase* GameModeBase;

	TMap<FGameplayTag, float> Resources;

};
