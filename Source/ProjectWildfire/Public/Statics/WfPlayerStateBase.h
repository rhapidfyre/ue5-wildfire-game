// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/WfFfCharacterBase.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerState.h"
#include "Saves/WfCharacterSaveGame.h"
#include "Vehicles/WfFireApparatusBase.h"

#include "WfPlayerStateBase.generated.h"


class AWfCalloutActor;
class AAIController;
class AWfGameModeBase;
class AWfFireStationBase;
class UWfFirefighterSaveGame;


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FFleetPurchaseData : public FTableRowBase
{
	GENERATED_BODY()

	FFleetPurchaseData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	UTexture2D* DisplayIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	FName ApparatusCallsign;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	TSubclassOf<AWfFireApparatusBase> FireApparatusType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Data")
	float PurchaseValue;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourceUpdated, const FGameplayTag&, ResourceTag, const float, OldValue, const float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFireApparatusPurchaseFail, const FText&, FailureReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnIncidentReceived, const AWfCalloutActor*, CalloutActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnIncidentExpired, const AWfCalloutActor*, CalloutActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFireStationChanged, const AWfFireStationBase*, NewFireStation, const bool, bIsOwned);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnApparatusAssignment, const AWfFireApparatusBase*, FireApparatus, const AWfCalloutActor*, CalloutActor);


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
	AWfFireStationBase* GetFireStationReference() const;

	UFUNCTION(BlueprintCallable)
	void PurchaseFireApparatus(const FFleetPurchaseData& PurchaseData);

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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	UFUNCTION(Server, Reliable)
	void Server_AcceptJobContract(const FJobContractData& JobContract);

	UFUNCTION(Server, Reliable)
	void Server_SetFireStationReference(AWfFireStationBase* FireStation);

	UFUNCTION(Server, Reliable)
	void Server_PurchaseFireApparatus(const FFleetPurchaseData& PurchaseData);

	virtual void SetupInitialResourceValues();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_PurchaseError(const FText& ErrorReason);

	virtual void SetupListeners();

private:

	UFUNCTION(BlueprintCallable)
	void SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue = 0.0f);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_FireStationBase(const AWfFireStationBase* OldFireStation);


	UFUNCTION(BlueprintCallable)
	void SetMoney(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetKilowattUsage(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetOxygenReserve(float NewValue = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetWaterStorage(float NewValue = 0.0f);

public:

	UPROPERTY(BlueprintAssignable)	FOnResourceUpdated OnResourceUpdated;
	UPROPERTY(BlueprintAssignable)	FOnFireStationChanged OnFireStationChanged;
	UPROPERTY(BlueprintAssignable)	FOnFireApparatusPurchaseFail OnFireApparatusPurchaseFail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	TSubclassOf<AAIController> UsingAiController;

private:

	// The fire station that the player owns
	UPROPERTY(ReplicatedUsing=OnRep_FireStationBase)
	AWfFireStationBase* FireStationBase;

	TMap<FGameplayTag, float> Resources;

};
