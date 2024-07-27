// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WfEquipmentData.h"
#include "UObject/Object.h"
#include "WfCalloutData.generated.h"

class AWfFfCharacterBase;
class AWfCalloutActor;
class AWfFireApparatusBase;

class AWfPropertyActor;
class AWfVehicleBase;

DECLARE_LOG_CATEGORY_EXTERN(LogCallouts, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalloutDispatchInitial, const AWfCalloutActor*, CalloutActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalloutDispatchFull, const AWfCalloutActor*, CalloutActor);

UENUM(BlueprintType)
enum class EIncidentType : uint8
{
	Medical = 0,
	Structure,
	Vegetation,
	Collision,
	HazMat,
	HighVoltage,
	Gas
};


// Data about equipment usage when managing a task
USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutEquipmentUse
{
	GENERATED_BODY()

	FCalloutEquipmentUse();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Equipment Usage")
	TSubclassOf<UEquipmentDataAsset> EquipmentUsed;

	// If true, the task does not consume the item, it just requires it.
	// i.e: ECG Monitor, Hose Nozzle
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Equipment Usage")
	bool bNoConsume;

	// The total amount of items to consume to reduce the single task from 100% to 0%
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Equipment Usage")
	float TotalUsageValue;
};


// Data about a single spot of fire
USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutDataFire
{
	GENERATED_BODY()

	FCalloutDataFire();

	// The current health value of this fire spot, where '0' is extinguished
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float TaskProgress;

	/**
	 * \brief The difficulty of the fire, which modifies WaterUsage and fire health regeneration.
	 *   \n 0 (0%) is 'Easy', where the fire is almost instantly extinguished
	 *   \n 1 (100%) is 'Hard', where the fire is very resilient and heals itself
	 *   \n Values < 0 or > 1 are treated as 0 or 1 (respective)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float Difficulty;

	// Gallons of water 'used' to reduce the health from 1 (100%) to 0 (0%)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float WaterUsage;

	// Equipment Required for Extinguishing this Fire Spot
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	TArray<FCalloutEquipmentUse> EquipmentUsage;

};


// Data about a single medical patient
USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutDataMedical
{
	GENERATED_BODY()

	FCalloutDataMedical();

	// The current illness value of this fire spot, where '0' is stabilized
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float TaskProgress;

	// The chance the patient will refuse transportation
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float RefusalChance;

	/**
	 * \brief The difficulty of the patient's medical condition.
	 * \n 0 (0%) is 'Easy', where the patient is quickly treated and transported, or may even refuse transport
	 * \n 1 (100%) is 'Hard', where the patient may take a long time or a lot of equipment to treat.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	float Difficulty;

	// Equipment Used when treating patients (decreasing illness) from 100% to 0%
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callout Fire Data")
	TArray<FCalloutEquipmentUse> EquipmentUsage;
};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutAssignment
{
	GENERATED_BODY()

	FCalloutAssignment();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") AWfFireApparatusBase* AssignedVehicle;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") AWfFfCharacterBase* AssignedCharacter;
};

USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutUnits
{
	GENERATED_BODY()

	FCalloutUnits();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts")
	TSubclassOf<AWfVehicleBase> ResourceUnitType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts")
	int QuantityMinimum;

};

/**
 * \brief Data Table Static Storage - For active callouts, use FCalloutData
 * This is NOT for dynamic scenarios. This is for static data-driven callouts.
 */
USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCallouts : public FTableRowBase
{
	GENERATED_BODY()

	FCallouts();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FString DisplayName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") UTexture2D* DisplayIcon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 AlertLevel; // Dispatch Priority
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float Payment; // On Success, to each player
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float Penalty; // On success, to each player

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float DifficultyMin; // 0 (0%) 1 (100%)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float DifficultyMax; // 0 (0%) 1 (100%)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float DifficultyVariance; // +/- <= 1.0f

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 PatientsMin;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 PatientsMax;

	// The difficulty percentage at which the patient will no longer refuse transportation
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float RefusalThreshold;

	// The maximum chance (below the refusal threshold) that a patient will refuse care
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") float RefusalChanceMax;

	// Equipment needed to run the call
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") TArray<FCalloutEquipmentUse> MedicalEquipment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 MinFires;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	 MaxFires;
	// Equipment needed to run the call
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") TArray<FCalloutEquipmentUse> FireEquipment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	DeadlineDays; // Days until penalized
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	DeadlineHours; // Hrs until penalized
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") int	DeadlineMinutes; // Mins until penalized

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") EIncidentType TypeOfIncident;

	// The dispatch phrase that describes this call out ("cardiac", "hazmat", etc.)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FName VoxPhrases;

	// The Text-to-Speech Phrase used when speaking this callout type (if VoxPhrase isn't found/used)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FString TtsSentence;

	// The minimum units required to answer this call.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") TArray<FCalloutUnits> MinimumUnits;
};

/**
 * \brief Active callouts. For data storage, use FCallouts
 * This is NOT for dynamic scenarios. This is for static data-driven callouts.
 */
USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FCalloutData : public FTableRowBase
{
	GENERATED_BODY()

	FCalloutData();
	explicit FCalloutData(const FCallouts& NewCallout);

	// An array of messages containing any failures or notices while generating the callout
	// Each string starts with "E", "L", or "N" - Error, Log Only, or Notice.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") TSubclassOf<AWfCalloutActor> CalloutActor;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Callouts") TArray<FString> CallLogging;

	// The UTC time on the server that the call was officially dispatched (for sync)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Callouts") FDateTime ServerTimeStart;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Callouts") float SecondsToStart;

	// Static Callout Data
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FCallouts CalloutData;

	// The location of the callout
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") AWfPropertyActor* PropertyActor;

	// The deadline for the call to be satisfied
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Callouts") FDateTime ResolutionDeadline;
};


/**
 * \brief Spawned when a callout is created, and manages all further actions of the callout.
 * The AWfCalloutsManager just handles generating callouts. This handles the callout itself.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfCalloutActor : public AActor
{
	GENERATED_BODY()

public:

	AWfCalloutActor();

	UFUNCTION(BlueprintPure)
	int GetIncidentNumber() const { return IncidentNumber; }

	void SetIncidentNumber(const int NewNumber) { IncidentNumber = NewNumber; }

	void SetCalloutData(FCallouts& NewCallout, const float SecondsToStart = 30.0f);

	UFUNCTION(BlueprintCallable)
	bool StartCallout();

	FDateTime GetGameDateTime() const;

	UFUNCTION(BlueprintCallable)
	void AssignUnitToCallout(AWfFireApparatusBase* FireVehicle, AWfFfCharacterBase* FireFighter);

	UFUNCTION(BlueprintPure)
	TArray<FCalloutAssignment> GetAssignedUnits() const { return AssignedUnits; }

	UFUNCTION(BlueprintPure)
	FCalloutData GetCalloutData() const { return CalloutData; }

protected:

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Penalize the players (assigned, but resolution time has passed)
	virtual void ApplyExpiredPenalty();

	// Perform the initial dispatch (pre-alert and unit assignment)
	virtual void DispatchInitial();

	// Perform the full dispatch (handles auto-dispatching)
	virtual void DispatchCallout();


private:

	UFUNCTION() void CheckCalloutExpired();
	UFUNCTION() void CalloutTick();
	UFUNCTION(NetMulticast, Reliable) void Multicast_DispatchPreAlert();
	UFUNCTION(NetMulticast, Reliable) void Multicast_DispatchCallout();

public:

	UPROPERTY(BlueprintAssignable) FOnCalloutDispatchInitial OnCalloutDispatchInitial;
	UPROPERTY(BlueprintAssignable) FOnCalloutDispatchFull OnCalloutDispatchFull;

private:

	// Data pertinent to this callout
	UPROPERTY(Replicated) FCalloutData CalloutData;

	FTimerHandle CalloutTimer;

	FTimerHandle ExpirationTimer;

	int IncidentNumber;

	// Once set to true, the callout cannot be modified.
	bool bCalloutReady = false;

	UPROPERTY()	TArray<FCalloutAssignment> AssignedUnits;

};
