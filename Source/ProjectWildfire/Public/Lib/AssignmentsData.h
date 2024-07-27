// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AssignmentsData.generated.h"


class AWfCalloutActor;
class AWfFireStationBase;
class AWfFfCharacterBase;
class AWfFireApparatusBase;


USTRUCT(BlueprintType, Blueprintable)
struct PROJECTWILDFIRE_API FFirefighterAssignments
{
	GENERATED_BODY()
	FFirefighterAssignments();
	explicit FFirefighterAssignments(AWfFfCharacterBase* NewFireFighter);
	bool operator==(const FFirefighterAssignments& CompareAssignment) const;
	bool operator==(const AWfFfCharacterBase* CompareActor) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter") AWfFfCharacterBase* Firefighter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter") AWfFireApparatusBase* FireApparatus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter") AWfFireStationBase* HomeFireStation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firefighter") AWfCalloutActor* Incident;
};

USTRUCT(BlueprintType, Blueprintable)
struct PROJECTWILDFIRE_API FFireStationAssignments
{
	GENERATED_BODY()
	FFireStationAssignments();
	explicit FFireStationAssignments(AWfFireStationBase* NewStation);
	bool operator==(const FFireStationAssignments& CompareAssignment) const;
	bool operator==(const AWfFireStationBase* CompareActor) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Station") AWfFireStationBase* FireStation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Station") TArray<AWfFireApparatusBase*> FireApparatuses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Station") TArray<AWfFfCharacterBase*> Firefighters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Station") TArray<AWfCalloutActor*> Incidents;
};

USTRUCT(BlueprintType, Blueprintable)
struct PROJECTWILDFIRE_API FFireApparatusAssignments
{
	GENERATED_BODY()
	FFireApparatusAssignments();
	explicit FFireApparatusAssignments(AWfFireApparatusBase* NewFireFighter);
	bool operator==(const FFireApparatusAssignments& CompareAssignment) const;
	bool operator==(const AWfFireApparatusBase* CompareActor) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Apparatus") AWfFireApparatusBase* FireApparatus = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Apparatus") TArray<AWfFfCharacterBase*> Firefighters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Apparatus") AWfFireStationBase* FireStation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Apparatus") AWfCalloutActor* Incident;
};

USTRUCT(BlueprintType, Blueprintable)
struct PROJECTWILDFIRE_API FIncidentAssignments
{
	GENERATED_BODY()
	FIncidentAssignments();
	explicit FIncidentAssignments(AWfCalloutActor* NewIncident);
	bool operator==(const FIncidentAssignments& CompareAssignment) const;
	bool operator==(const AWfCalloutActor* CompareActor) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident") AWfCalloutActor* Incident;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident") TArray<AWfFireApparatusBase*> FireApparatuses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident") TArray<AWfFfCharacterBase*> Firefighters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Incident") TArray<AWfFireStationBase*> FireStations;
};
