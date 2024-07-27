// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WfFireStationBase.h"
#include "Delegates/Delegate.h"
#include "Engine/DirectionalLight.h"
#include "Lib/AssignmentsData.h"

#include "GameManager.generated.h"

USTRUCT()
struct PROJECTWILDFIRE_API FSimDateTime
{
	GENERATED_BODY()

	FSimDateTime();

	float SimTimeRate;			// The current simulation time rate (simulation seconds per real life second)

	FDateTime SimDateTime;		// The current simulated time
	FDateTime SyncDateTime;		// The UTC time when this update was made on the server
	FDateTime SimTimeStart;		// The UTC time when the game was first started
	FTimespan TimeSinceStart;	// The amount of time that has passed since the game initially started

};


DECLARE_LOG_CATEGORY_EXTERN(LogManager, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeSynchronized, const FDateTime&, SimulatedDateTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeRateChanged, const float, SimulationTimeRate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirefighterHired, const AWfFfCharacterBase*, Firefighter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFirefighterFired, const AWfFfCharacterBase*, Firefighter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFirefighterUpdated,
	const FFirefighterAssignments&, OldAssignment, const FFirefighterAssignments&, NewAssignment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireApparatusSpawned, const AWfFireApparatusBase*, FireApparatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireApparatusDestroyed, const AWfFireApparatusBase*, FireApparatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireApparatusUpdated,
	const FFireApparatusAssignments&, OldAssignment, const FFireApparatusAssignments&, NewAssignment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireStationSpawned, const AWfFireStationBase*, FireStation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireStationDestroyed, const AWfFireStationBase*, FireStation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireStationUpdated,
	const FFireStationAssignments&, OldAssignment, const FFireStationAssignments&, NewAssignment);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIncidentStarted, const AWfCalloutActor*, IncidentActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIncidentEnded, const AWfCalloutActor*, IncidentActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIncidentUpdated,
	const FIncidentAssignments&, OldAssignment, const FIncidentAssignments&, NewAssignment);

UCLASS(BlueprintType)
class PROJECTWILDFIRE_API AGameManager : public AActor
{
	GENERATED_BODY()

public:

	AGameManager();

	UFUNCTION(BlueprintCallable)
	void SetSimulatedTimeRate(const float NewTimeRate = 1.0f);

	UFUNCTION(BlueprintPure)
	float GetSimulatedTimeRate() const { return SynchronizedTime.SimTimeRate; }

	UFUNCTION(BlueprintPure)
	FDateTime GetSimulatedDateTime() const { return CurrentSimTime.SimDateTime; }

	// Function to get the instance
	UFUNCTION(BlueprintCallable, Category = "Game Manager Singleton", meta = (WorldContext = "WorldContextObject"))
	static AGameManager* GetInstance(UObject* WorldContext);

	UFUNCTION(BlueprintPure)
	TArray<FFireStationAssignments> GetPlayerFireStationAssignments(const APlayerController* PlayerController);

	UFUNCTION(BlueprintPure)
	TArray<FFireApparatusAssignments> GetAllPlayerFireApparatuses(const APlayerController* PlayerController);

	UFUNCTION(BlueprintPure)
	TArray<FFirefighterAssignments> GetAllPlayerFirefighters(const APlayerController* PlayerController);

    // Incident Actor Spawned: Update existing or create AssignedIncidents entry
    void AddIncidentActor(AWfCalloutActor* IncidentActor);

	UFUNCTION(BlueprintPure)
	FIncidentAssignments GetIncidentAssignments(const AWfCalloutActor* CalloutActor) const;

	UFUNCTION(BlueprintPure)
	TArray<FIncidentAssignments> GetAllIncidentAssignments() const;

    // Incident Actor Destroyed: Delete the associated AssignedIncidents entry
    void RemoveIncidentActor(AWfCalloutActor* IncidentActor);

    // Fire Station Added: Update existing AssignedFireStations or create a new one
	void AddFireStation(AWfFireStationBase* FireStation);

	UFUNCTION(BlueprintPure)
	FFireStationAssignments GetFireStationAssignments(const AWfFireStationBase* FireStation) const;

	UFUNCTION(BlueprintPure)
	TArray<FFireStationAssignments> GetAllFireStationAssignments() const;

    // Fire Station Removed: Delete existing AssignedFireStations if exists
    void RemoveFireStation(AWfFireStationBase* FireStation);

    // Fire Apparatus Spawned: Update existing or create FFireApparatusAssignments entry
	void AddFireApparatus(AWfFireApparatusBase* FireApparatus);

	UFUNCTION(BlueprintPure)
	int GetNextApparatusNumber(const AWfFireStationBase* FireStation, const FString& ApparatusType) const;

	UFUNCTION(BlueprintPure)
	FFireApparatusAssignments GetFireApparatusAssignments(const AWfFireApparatusBase* FireApparatus) const;

	UFUNCTION(BlueprintPure)
	TArray<FFireApparatusAssignments> GetAllFireApparatusAssignments() const;

    // Fire Apparatus Destroyed: Delete the associated FFireApparatusAssignments entry
    void RemoveFireApparatus(AWfFireApparatusBase* FireApparatus);

    // Firefighter Spawned: Update existing or create AssignedFirePersonnel entry
	void AddFirefighter(AWfFfCharacterBase* Firefighter);

	UFUNCTION(BlueprintPure)
	FFirefighterAssignments GetFirefighterAssignments(const AWfFfCharacterBase* Firefighter) const;

	UFUNCTION(BlueprintPure)
	TArray<FFirefighterAssignments> GetAllFirefighterAssignments() const;

    // Firefighter Destroyed: Delete the associated AssignedFirePersonnel entry
    void RemoveFirefighter(AWfFfCharacterBase* Firefighter);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
    void AssignIncidentToFireApparatus(
    	AWfCalloutActor* IncidentActor, AWfFireApparatusBase* FireApparatus);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
    void AssignIncidentToFirefighter(
    	AWfCalloutActor* IncidentActor, AWfFfCharacterBase* Firefighter);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
	void AssignIncidentToFireStation(
		AWfCalloutActor* IncidentActor, AWfFireStationBase* FireStation);

	UFUNCTION(BlueprintCallable, Category = "Apparatus Management")
	void AssignFirefighterToFireApparatus(
		AWfFireApparatusBase* FireApparatus, AWfFfCharacterBase* Firefighter);

	UFUNCTION(BlueprintCallable, Category = "Firefighter Management")
	void AssignFirefighterToFireStation(
		AWfFfCharacterBase* Firefighter, AWfFireStationBase* FireStation);

	UFUNCTION(BlueprintCallable, Category = "Apparatus Management")
	void AssignFireApparatusToFireStation(
		AWfFireApparatusBase* FireApparatus, AWfFireStationBase* FireStation);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
    void UnassignIncidentFromApparatus(
    	AWfCalloutActor* IncidentActor, AWfFireApparatusBase* FireApparatus);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
    void UnassignIncidentFromFirefighter(
    	AWfCalloutActor* IncidentActor, AWfFfCharacterBase* Firefighter);

    UFUNCTION(BlueprintCallable, Category = "Incident Management")
    void UnassignIncidentFromFireStation(
    	AWfCalloutActor* IncidentActor, AWfFireStationBase* FireStation);

    UFUNCTION(BlueprintCallable, Category = "Apparatus Management")
    void UnassignFirefighterFromApparatus(
    	AWfFireApparatusBase* FireApparatus, AWfFfCharacterBase* Firefighter);

protected:

	void Initialize();

	void SyncSimTime();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void InitializeAssignments(const TSubclassOf<AActor>& ClassType);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_SynchronizedTime(const FSimDateTime& OldSimDateTime);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_AssignedFireApparatuses(const TArray<FFireApparatusAssignments>& OldAssignments);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_AssignedFirePersonnel(const TArray<FFirefighterAssignments>& OldAssignments);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_AssignedFireStations(const TArray<FFireStationAssignments>& OldAssignments);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_AssignedIncidents(const TArray<FIncidentAssignments>& OldAssignments);

public:

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable) FOnTimeSynchronized OnTimeSynchronized;
	UPROPERTY(BlueprintAssignable) FOnTimeRateChanged OnTimeRateChanged;
	UPROPERTY(BlueprintAssignable) FOnFirefighterHired OnFirefighterHired;
	UPROPERTY(BlueprintAssignable) FOnFirefighterFired OnFirefighterFired;
    UPROPERTY(BlueprintAssignable) FOnFirefighterUpdated OnFirefighterUpdated;
	UPROPERTY(BlueprintAssignable) FOnFireApparatusSpawned OnFireApparatusSpawned;
	UPROPERTY(BlueprintAssignable) FOnFireApparatusDestroyed OnFireApparatusDestroyed;
    UPROPERTY(BlueprintAssignable) FOnFireApparatusUpdated OnFireApparatusUpdated;
	UPROPERTY(BlueprintAssignable) FOnFireStationSpawned OnFireStationSpawned;
	UPROPERTY(BlueprintAssignable) FOnFireStationDestroyed OnFireStationDestroyed;
    UPROPERTY(BlueprintAssignable) FOnFireStationUpdated OnFireStationUpdated;
	UPROPERTY(BlueprintAssignable) FOnIncidentStarted OnIncidentStarted;
	UPROPERTY(BlueprintAssignable) FOnIncidentEnded OnIncidentEnded;
    UPROPERTY(BlueprintAssignable) FOnIncidentUpdated OnIncidentUpdated;

private:

	void AdjustSunLight();

	// The last time the server synchronized, in UTC
	UPROPERTY(ReplicatedUsing=OnRep_SynchronizedTime) FSimDateTime SynchronizedTime;
	FSimDateTime CurrentSimTime;

	FTimerHandle SimTimerHandle;

	UPROPERTY() ADirectionalLight* DirectionalLight;

	// The singleton instance
	static AGameManager* Instance;

	float SyncSeconds = 10.0f;
	float MaxSimRate  = 3600.0f;

	UPROPERTY(ReplicatedUsing=OnRep_AssignedFireApparatuses) TArray<FFireApparatusAssignments> AssignedFireApparatuses;
	UPROPERTY(ReplicatedUsing=OnRep_AssignedFirePersonnel) TArray<FFirefighterAssignments>   AssignedFirePersonnel;
	UPROPERTY(ReplicatedUsing=OnRep_AssignedFireStations) TArray<FFireStationAssignments>   AssignedFireStations;
	UPROPERTY(ReplicatedUsing=OnRep_AssignedIncidents) TArray<FIncidentAssignments>      AssignedIncidents;
};
