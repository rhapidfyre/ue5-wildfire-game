// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GameManager.h"

#include "EngineUtils.h"
#include "Actors/WfFireStationBase.h"
#include "Characters/WfFfCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/WfCalloutData.h"
#include "Net/UnrealNetwork.h"
#include "Vehicles/WfFireApparatusBase.h"

DEFINE_LOG_CATEGORY(LogManager);

AGameManager* AGameManager::Instance = nullptr;


FSimDateTime::FSimDateTime()
    : SimTimeRate(0.0f), SimDateTime(0), SyncDateTime(0), SimTimeStart(0), TimeSinceStart(0)
{
}

AGameManager::AGameManager()
{
    PrimaryActorTick.bCanEverTick = true;
    DirectionalLight = nullptr;
    bReplicates = true;
}

void AGameManager::SetSimulatedTimeRate(const float NewTimeRate)
{
    if (!HasAuthority())
        return;

    if (FMath::IsNearlyEqual(SynchronizedTime.SimTimeRate, NewTimeRate, 0.0001))
        return;

    CurrentSimTime.SimTimeRate = FMath::Clamp(NewTimeRate, 0.0f, MaxSimRate);

    SyncSimTime();
    if (SimTimerHandle.IsValid())
        SimTimerHandle.Invalidate();
    GetWorldTimerManager().SetTimer(SimTimerHandle,
        this, &AGameManager::SyncSimTime, SyncSeconds, true);

    UE_LOGFMT(LogManager, Warning, "Simulated Time has been Changed to 'x {TimeRate}'.", CurrentSimTime.SimTimeRate);
}

/**
 * \brief Initialize assignments by checking for existing entities in the world
 * This is called in BeginPlay() to ensure the GameManager is aware of all current actors.
 */
void AGameManager::InitializeAssignments(const TSubclassOf<AActor>& ClassType)
{
    TArray<AActor*> OutActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ClassType, OutActors);
    for (const auto& OutActor : OutActors)
    {
    	if (AWfFireStationBase* FireStation = Cast<AWfFireStationBase>(OutActor))
    	{
    		if (!AssignedFireStations.Contains(FireStation))
    		{
    		    AssignedFireStations.Add(FFireStationAssignments(FireStation));
                UE_LOGFMT(LogManager, Display,
                    "Fire Station {FireStation} added to fire stations array (spawned before initialization).", FireStation->GetFireStationNumber());
    		}
    		continue;
    	}
    	if (AWfFireApparatusBase* FireApparatus = Cast<AWfFireApparatusBase>(OutActor))
    	{
    		if (!AssignedFireApparatuses.Contains(FireApparatus))
            {
    		    AssignedFireApparatuses.Add(FFireApparatusAssignments(FireApparatus));
    		    UE_LOGFMT(LogManager, Display,
                    "Fire Apparatus {FireApparatus} added to fire apparatus array (spawned before initialization).", FireApparatus->GetApparatusIdentity());
            }
    		continue;
    	}
    	if (AWfFfCharacterBase* Firefighter = Cast<AWfFfCharacterBase>(OutActor))
    	{
    		if (!AssignedFirePersonnel.Contains(Firefighter))
            {
    		    AssignedFirePersonnel.Add(FFirefighterAssignments(Firefighter));
    		    UE_LOGFMT(LogManager, Display,
                    "Firefighter '{CharName}' added to firefighters array (spawned before initialization).", Firefighter->GetCharacterName());
            }
            continue;
    	}
    	if (AWfCalloutActor* IncidentActor = Cast<AWfCalloutActor>(OutActor))
    	{
    		if (!AssignedIncidents.Contains(IncidentActor))
            {
    		    AssignedIncidents.Add(FIncidentAssignments(IncidentActor));
    		    UE_LOGFMT(LogManager, Display,
                    "Incident '{IncidentActor}' added to incidents array (spawned before initialization).", IncidentActor->GetIncidentNumber());
            }
        }
    }
}

void AGameManager::OnRep_SynchronizedTime_Implementation(const FSimDateTime& OldSimDateTime)
{
    // If the simulation time rate has changed
    if (!FMath::IsNearlyEqual(CurrentSimTime.SimTimeRate, SynchronizedTime.SimTimeRate, 0.0001f))
    {
        UE_LOGFMT(LogManager, Display,
            "({NetMode}) Simulation Time Rate Changed from '{FromRate}' to '{ToRate}'", CurrentSimTime.SimTimeRate, SynchronizedTime.SimTimeRate);
        if (OnTimeRateChanged.IsBound())
            OnTimeRateChanged.Broadcast(SynchronizedTime.SimTimeRate);
    }

    // Determine the difference in local time from server time, and adjust accordingly
    CurrentSimTime = SynchronizedTime;
    const FDateTime LocalTimeUtc = FDateTime::UtcNow();
    const FTimespan TimeLatency  = LocalTimeUtc - SynchronizedTime.SyncDateTime;
    CurrentSimTime.SimDateTime = SynchronizedTime.SimDateTime + TimeLatency;

    UE_LOGFMT(LogManager, Display, "({NetMode}) Simulation Time Sync: {SimTime}"
        , HasAuthority() ? "SERVER" : "CLIENT", CurrentSimTime.SimDateTime.ToString());

    if (OnTimeSynchronized.IsBound())
        OnTimeSynchronized.Broadcast(CurrentSimTime.SimDateTime);
}

AGameManager* AGameManager::GetInstance(UObject* WorldContext)
{
    if (Instance == nullptr)
    {
        if (WorldContext)
        {
            if (UWorld* World = WorldContext->GetWorld())
            {
                for (TActorIterator<AGameManager> It(World); It; ++It)
                {
                    return *It;
                }

                Instance = World->SpawnActor<AGameManager>();
            }
        }
    }
    return Instance;
}

TArray<FFireStationAssignments> AGameManager::GetPlayerFireStationAssignments(const APlayerController* PlayerController)
{
    if (!HasAuthority())
        return {};
    if (!IsValid(PlayerController))
        return {};
    TArray<FFireStationAssignments> AllAssignments;
    for (auto& AssignmentData : AssignedFireStations)
    {
        if (AssignmentData.FireStation->GetOwner() == PlayerController)
            AllAssignments.Add(AssignmentData);
    }
    return AllAssignments;
}

TArray<FFireApparatusAssignments> AGameManager::GetAllPlayerFireApparatuses(const APlayerController* PlayerController)
{
    if (!HasAuthority())
        return {};
    if (!IsValid(PlayerController))
        return {};
    TArray<FFireApparatusAssignments> AllAssignments;
    for (auto& AssignmentData : AssignedFireApparatuses)
    {
        if (AssignmentData.FireApparatus->GetOwner() == PlayerController)
            AllAssignments.Add(AssignmentData);
    }
    return AllAssignments;
}

TArray<FFirefighterAssignments> AGameManager::GetAllPlayerFirefighters(const APlayerController* PlayerController)
{
    if (!HasAuthority())
        return {};
    if (!IsValid(PlayerController))
        return {};
    TArray<FFirefighterAssignments> AllAssignments;
    for (auto& AssignmentData : AssignedFirePersonnel)
    {
        if (AssignmentData.Firefighter->GetOwner() == PlayerController)
            AllAssignments.Add(AssignmentData);
    }
    return AllAssignments;
}

/**
 * \brief Incident Actor Spawned: Update existing or create AssignedIncidents entry
 */
void AGameManager::AddIncidentActor(AWfCalloutActor* IncidentActor)
{
    if (!HasAuthority()) return;
    if (!AssignedIncidents.Contains(IncidentActor))
    {
        AssignedIncidents.Add(FIncidentAssignments(IncidentActor));
        UE_LOGFMT(LogManager, Display, "Tracking New Incident: {IncidentName} (# {IncidentNum})"
            , IncidentActor->GetName(), IncidentActor->GetIncidentNumber());
        if (OnIncidentStarted.IsBound())
            OnIncidentStarted.Broadcast(IncidentActor);
    }
}

FIncidentAssignments AGameManager::GetIncidentAssignments(const AWfCalloutActor* CalloutActor) const
{
    for (auto& AssignmentData : GetAllIncidentAssignments())
    {
        if (IsValid(AssignmentData.Incident))
        {
            if (AssignmentData.Incident == CalloutActor)
                return AssignmentData;
        }
    }
    return {};
}

TArray<FIncidentAssignments> AGameManager::GetAllIncidentAssignments() const
{
    return AssignedIncidents;
}

/**
 * \brief Incident Actor Destroyed: Delete the associated AssignedIncidents entry
 */
void AGameManager::RemoveIncidentActor(AWfCalloutActor* IncidentActor)
{
    AssignedIncidents.RemoveAll([IncidentActor](const FIncidentAssignments& Assignment)
    {
        return Assignment.Incident == IncidentActor;
    });
    if (!AssignedIncidents.Contains(IncidentActor))
    {
        UE_LOGFMT(LogManager, Display, "Incident Removed: {IncidentName} (# {IncidentNum})"
        , IncidentActor->GetName(), IncidentActor->GetIncidentNumber());
        if (OnIncidentEnded.IsBound())
            OnIncidentEnded.Broadcast(IncidentActor);
    }
    else
    {
        UE_LOGFMT(LogManager, Error, "Failed to remove incident '{IncidentName}' (# {IncidentNum})"
            , IncidentActor->GetName(), IncidentActor->GetIncidentNumber());
    }
}

/**
 * \brief Fire Station Added: Update existing AssignedFireStations or create a new one
 */
void AGameManager::AddFireStation(AWfFireStationBase* FireStation)
{
    if (!HasAuthority()) return;
    if (!AssignedFireStations.Contains(FireStation))
    {
        AssignedFireStations.Add(FFireStationAssignments(FireStation));
        UE_LOGFMT(LogManager, Display, "Tracking New Fire Station: {StationName} (Fire Station #{StationNum})"
            , FireStation->GetName(), FireStation->GetFireStationNumber());
    }
}

FFireStationAssignments AGameManager::GetFireStationAssignments(const AWfFireStationBase* FireStation) const
{
    for (auto& AssignmentData : GetAllFireStationAssignments())
    {
        if (IsValid(AssignmentData.FireStation))
        {
            if (AssignmentData.FireStation == FireStation)
                return AssignmentData;
        }
    }
    return {};
}

TArray<FFireStationAssignments> AGameManager::GetAllFireStationAssignments() const
{
    return AssignedFireStations;
}

/**
 * \brief Fire Station Removed: Delete existing AssignedFireStations if exists
 */
void AGameManager::RemoveFireStation(AWfFireStationBase* FireStation)
{
    AssignedFireStations.RemoveAll([FireStation](const FFireStationAssignments& Assignment)
    {
        return Assignment.FireStation == FireStation;
    });
    if (!AssignedFireStations.Contains(FireStation))
    {
        UE_LOGFMT(LogManager, Display, "Fire Station Removed: {StationName} (Fire Station #{StationNum})"
            , FireStation->GetName(), FireStation->GetFireStationNumber());
    }
    else
    {
        UE_LOGFMT(LogManager, Error, "Failed to remove Fire Station '{StationName}' (Fire Station #{StationNum})"
            , FireStation->GetName(), FireStation->GetFireStationNumber());
    }
}

/**
 * \brief Fire Apparatus Spawned: Update existing or create FFireApparatusAssignments entry
 */
void AGameManager::AddFireApparatus(AWfFireApparatusBase* FireApparatus)
{
    if (!HasAuthority()) return;
    if (!AssignedFireApparatuses.Contains(FireApparatus))
    {
        AssignedFireApparatuses.Add(FFireApparatusAssignments(FireApparatus));
        UE_LOGFMT(LogManager, Display, "Tracking New Fire Apparatus: {ApparatusName} ({ApparatusIdentity})"
            , FireApparatus->GetName(), FireApparatus->GetApparatusIdentity());
    }
}

/**
 * \brief Gets the next available apparatus number, filling in starting from '1'
 * \param FireStation The fire station in reference
 * \param ApparatusType The apparatus type identifier (medic, engine, copter, etc)
 * \return
 */
int AGameManager::GetNextApparatusNumber(const AWfFireStationBase* FireStation, const FString& ApparatusType) const
{
    if (!IsValid(FireStation))
        return 0;

    int NextNumber = 1;
    TSet<int> NumbersTaken;
    FFireStationAssignments AssignmentData = GetFireStationAssignments(FireStation);
    for (const auto& FireApparatus : AssignmentData.FireApparatuses)
    {
        if (FireApparatus->GetApparatusIdentityType() == ApparatusType)
            NumbersTaken.Add(FireApparatus->GetApparatusIdentityUnique());
    }
    while (NumbersTaken.Contains(NextNumber)) { NextNumber++; }
    return NextNumber;
}

FFireApparatusAssignments AGameManager::GetFireApparatusAssignments(const AWfFireApparatusBase* FireApparatus) const
{
    for (auto& AssignmentData : GetAllFireApparatusAssignments())
    {
        if (IsValid(AssignmentData.FireApparatus))
        {
            if (AssignmentData.FireApparatus == FireApparatus)
                return AssignmentData;
        }
    }
    return {};
}

TArray<FFireApparatusAssignments> AGameManager::GetAllFireApparatusAssignments() const
{
    return AssignedFireApparatuses;
}

/**
 * \brief Fire Apparatus Destroyed: Delete the associated FFireApparatusAssignments entry
 */
void AGameManager::RemoveFireApparatus(AWfFireApparatusBase* FireApparatus)
{
    AssignedFireApparatuses.RemoveAll([FireApparatus](const FFireApparatusAssignments& Assignment)
    {
        return Assignment.FireApparatus == FireApparatus;
    });
    if (!AssignedFireApparatuses.Contains(FireApparatus))
    {
        UE_LOGFMT(LogManager, Display, "Fire Apparatus Removed: {FireApparatusName} (Fire Station #{AppIdentity})"
            , FireApparatus->GetName(), FireApparatus->GetApparatusIdentity());
    }
    else
    {
        UE_LOGFMT(LogManager, Error, "Failed to remove Fire Apparatus '{FireApparatusName}' (Fire Station #{AppIdentity})"
            , FireApparatus->GetName(), FireApparatus->GetApparatusIdentity());
    }
}

/**
 * \brief Firefighter Spawned: Update existing or create AssignedFirePersonnel entry
 */
void AGameManager::AddFirefighter(AWfFfCharacterBase* Firefighter)
{
    if (!HasAuthority()) return;
    if (!AssignedFirePersonnel.Contains(Firefighter))
    {
        AssignedFirePersonnel.Add(FFirefighterAssignments(Firefighter));
        UE_LOGFMT(LogManager, Display, "Tracking New Firefighter: {ActorName} ({CharacterName})"
            , Firefighter->GetName(), Firefighter->GetCharacterName());
    }
}

FFirefighterAssignments AGameManager::GetFirefighterAssignments(const AWfFfCharacterBase* Firefighter) const
{
    for (const auto& AssignmentData : GetAllFirefighterAssignments())
    {
        if (IsValid(AssignmentData.Firefighter))
        {
            if (AssignmentData.Firefighter == Firefighter)
                return AssignmentData;
        }
    }
    return {};
}

TArray<FFirefighterAssignments> AGameManager::GetAllFirefighterAssignments() const
{
    return AssignedFirePersonnel;
}

/**
 * \brief Firefighter Destroyed: Delete the associated AssignedFirePersonnel entry
 */
void AGameManager::RemoveFirefighter(AWfFfCharacterBase* Firefighter)
{
    AssignedFirePersonnel.RemoveAll([Firefighter](const FFirefighterAssignments& Assignment)
    {
        return Assignment.Firefighter == Firefighter;
    });
    if (!AssignedFirePersonnel.Contains(Firefighter))
    {
        UE_LOGFMT(LogManager, Display, "Fire Apparatus Removed: {ActorName} ({CharacterName})"
            , Firefighter->GetName(), Firefighter->GetCharacterName());
    }
    else
    {
        UE_LOGFMT(LogManager, Error, "Failed to remove Fire Apparatus '{ActorName}' ({CharacterName})"
            , Firefighter->GetName(), Firefighter->GetCharacterName());
    }
}

/**
 * \brief Assign Fire Apparatus to Incident
 */
void AGameManager::AssignIncidentToFireApparatus(AWfCalloutActor* IncidentActor, AWfFireApparatusBase* FireApparatus)
{
    AddFireApparatus(FireApparatus);
    AddIncidentActor(IncidentActor);

    // Assign the incident to the specified apparatus and it's assigned personnel
    for (FFireApparatusAssignments& ApparatusAssignment : AssignedFireApparatuses)
    {
        if (ApparatusAssignment.FireApparatus == FireApparatus)
        {
            ApparatusAssignment.Incident = IncidentActor;
            UE_LOGFMT(LogManager, Display, "{ApparatusId} has been assigned to Incident #{IncidentNum}"
                , FireApparatus->GetApparatusIdentity(), IncidentActor->GetIncidentNumber());
            for (AWfFfCharacterBase* Firefighter : ApparatusAssignment.Firefighters)
            {
                AssignIncidentToFirefighter(IncidentActor, Firefighter);
            }
            break;
        }
    }

    // Assign the fire apparatus to the incident
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            if (!IncidentAssignment.FireApparatuses.Contains(FireApparatus))
            {
                IncidentAssignment.FireApparatuses.Add(FireApparatus);
                UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is now tracking {ApparatusId}"
                    , IncidentActor->GetIncidentNumber(), FireApparatus->GetApparatusIdentity());
            }
            break;
        }
    }
}

/**
 * \brief Assign Firefighter to Incident
 */
void AGameManager::AssignIncidentToFirefighter(AWfCalloutActor* IncidentActor, AWfFfCharacterBase* Firefighter)
{
    AddIncidentActor(IncidentActor);
    AddFirefighter(Firefighter);

    // Finds the specifies firefighter and adds the incident to it
    for (FFirefighterAssignments& FirefighterAssignment : AssignedFirePersonnel)
    {
        if (FirefighterAssignment.Firefighter == Firefighter)
        {
            FirefighterAssignment.Incident = IncidentActor;
            UE_LOGFMT(LogManager, Display, "{CharacterName} has been assigned to Incident #{IncidentNum}"
                , Firefighter->GetCharacterName(), IncidentActor->GetIncidentNumber());
            break;
        }
    }

    // Finds the specifies incident and adds the firefighter to it
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            if (!IncidentAssignment.Firefighters.Contains(Firefighter))
            {
                IncidentAssignment.Firefighters.Add(Firefighter);
                UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is now tracking {CharacterName}"
                    , IncidentActor->GetIncidentNumber(), Firefighter->GetCharacterName());
                break;
            }
        }
    }
}

/**
 * \brief Add the fire station as assigned to the incident,
 *      and adds the incident to the fire station's list of active incidents.
 */
void AGameManager::AssignIncidentToFireStation(AWfCalloutActor* IncidentActor, AWfFireStationBase* FireStation)
{
    AddIncidentActor(IncidentActor);
    AddFireStation(FireStation);

    // Assign the incident to the fire station's incident list
    for (FFireStationAssignments& StationAssignment : AssignedFireStations)
    {
        if (StationAssignment.FireStation == FireStation)
        {
            if (!StationAssignment.Incidents.Contains(IncidentActor))
            {
                StationAssignment.Incidents.Add(IncidentActor);
                UE_LOGFMT(LogManager, Display, "Fire Station #{StationNum} is now tracking Incident #{IncidentNum}"
                    , FireStation->GetFireStationNumber(), IncidentActor->GetIncidentNumber());
                break;
            }
        }
    }

    // Assign the fire station to the incident
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            if (!IncidentAssignment.FireStations.Contains(FireStation))
            {
                IncidentAssignment.FireStations.Add(FireStation);
                UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is now tracking Fire Station #{StationNum}"
                    , IncidentActor->GetIncidentNumber(), FireStation->GetFireStationNumber());
                break;
            }
        }
    }
}

/**
 * \brief Assigns the specified Firefighter to the specified Fire Apparatus
 */
void AGameManager::AssignFirefighterToFireApparatus(AWfFireApparatusBase* FireApparatus, AWfFfCharacterBase* Firefighter)
{
    bool bSuccessful = false;
    AddFireApparatus(FireApparatus);
    AddFirefighter(Firefighter);

    for (FFireApparatusAssignments& ApparatusAssignment : AssignedFireApparatuses)
    {
        if (ApparatusAssignment.FireApparatus == FireApparatus)
        {
            if (ApparatusAssignment.Firefighters.Num() < FireApparatus->NumberOfSeats)
            {
                ApparatusAssignment.Firefighters.Add(Firefighter);
                UE_LOGFMT(LogManager, Display, "{AppIdentity} is now tracking {CharacterName}"
                    , FireApparatus->GetApparatusIdentity(), Firefighter->GetCharacterName());
                bSuccessful = true;
                break;
            }
            UE_LOGFMT(LogManager, Error, "{AppIdentity} has no more seats available for firefighter assignments."
                , FireApparatus->GetApparatusIdentity(), Firefighter->GetCharacterName());
            return;
        }
    }

    if (bSuccessful)
    {
        for (FFirefighterAssignments& FirefighterAssignment : AssignedFirePersonnel)
        {
            if (FirefighterAssignment.Firefighter == Firefighter)
            {
                FirefighterAssignment.FireApparatus = FireApparatus;
                UE_LOGFMT(LogManager, Display, "{CharacterName} has been assigned to {AppIdentity}"
                    , Firefighter->GetCharacterName(), FireApparatus->GetApparatusIdentity());
                return;
            }
        }
    }
}

void AGameManager::AssignFirefighterToFireStation(AWfFfCharacterBase* Firefighter, AWfFireStationBase* FireStation)
{
    AddFirefighter(Firefighter);
    AddFireStation(FireStation);

    // Finds the specifies firefighter and adds the fire station to it
    for (FFirefighterAssignments& FirefighterAssignment : AssignedFirePersonnel)
    {
        if (FirefighterAssignment.Firefighter == Firefighter)
        {
            FirefighterAssignment.HomeFireStation = FireStation;
            UE_LOGFMT(LogManager, Display, "{CharacterName} has been assigned to Fire Station {StationNum}"
                , Firefighter->GetCharacterName(), FireStation->GetFireStationNumber());
            break;
        }
    }

    // Finds the specified fire station and adds the firefighter to it
    for (FFireStationAssignments& StationAssignments : AssignedFireStations)
    {
        if (StationAssignments.FireStation == FireStation)
        {
            if (!StationAssignments.Firefighters.Contains(Firefighter))
            {
                StationAssignments.Firefighters.Add(Firefighter);
                UE_LOGFMT(LogManager, Display, "Fire Station {StationNum} is now tracking {CharacterName}"
                    , FireStation->GetFireStationNumber(), Firefighter->GetCharacterName());
                break;
            }
        }
    }
}

/**
 * \brief Assign Fire Apparatus (Vehicle) to Fire Station, and sets the identifiers of the fire apparatus
 */
void AGameManager::AssignFireApparatusToFireStation(AWfFireApparatusBase* FireApparatus, AWfFireStationBase* FireStation)
{
    AddFireStation(FireStation);
    AddFireApparatus(FireApparatus);

    for (FFireStationAssignments& StationAssignment : AssignedFireStations)
    {
        if (StationAssignment.FireStation == FireStation)
        {
            if (!StationAssignment.FireApparatuses.Contains(FireApparatus))
            {
                StationAssignment.FireApparatuses.Add(FireApparatus);
                UE_LOGFMT(LogManager, Display, "Fire Station #{StationNum} is now tracking {AppIdentity}"
                    , FireStation->GetFireStationNumber(), FireApparatus->GetName());
                break;
            }
        }
    }

    for (FFireApparatusAssignments& ApparatusAssignments : AssignedFireApparatuses)
    {
        if (ApparatusAssignments.FireApparatus == FireApparatus)
        {
            ApparatusAssignments.FireStation = FireStation;
            UE_LOGFMT(LogManager, Display, "{AppIdentity} has been assigned to Fire Station #{StationNum}"
                , FireApparatus->GetName(), FireStation->GetFireStationNumber());
            break;
        }
    }

    FireApparatus->SetFireStationIdentity(FireStation->FireStationNumber);

    const FString ApparatusType = FireApparatus->GetApparatusIdentityType();
    FireApparatus->SetUniqueIdentity(GetNextApparatusNumber(FireStation, ApparatusType));
}

/**
 * \brief Unassign Incident from Fire Apparatus
 */
void AGameManager::UnassignIncidentFromApparatus(AWfCalloutActor* IncidentActor, AWfFireApparatusBase* FireApparatus)
{
    // Remove the apparatus from the incident
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            IncidentAssignment.FireApparatuses.Remove(FireApparatus);
            UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is no longer tracking {AppIdentity}"
                , IncidentActor->GetIncidentNumber(), FireApparatus->GetApparatusIdentity());
            for (AWfFfCharacterBase* Firefighter : IncidentAssignment.Firefighters)
            {
                UnassignIncidentFromFirefighter(IncidentActor, Firefighter);
            }
        }
    }

    // Set the apparatus' incident reference to nullptr (unassigned)
    for (FFireApparatusAssignments& ApparatusAssignment : AssignedFireApparatuses)
    {
        if (ApparatusAssignment.FireApparatus == FireApparatus)
        {
            ApparatusAssignment.Incident = nullptr;
            UE_LOGFMT(LogManager, Display, "{AppIdentity} is no longer assigned to an incident."
                , FireApparatus->GetApparatusIdentity());
        }
    }
}

/**
 * \brief Unassign Incident from Firefighter
 */
void AGameManager::UnassignIncidentFromFirefighter(AWfCalloutActor* IncidentActor, AWfFfCharacterBase* Firefighter)
{
    // Remove the firefighter from the incident
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            IncidentAssignment.Firefighters.Remove(Firefighter);
            UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is no longer tracking {CharacterName}"
                , IncidentActor->GetIncidentNumber(), Firefighter->GetCharacterName());
        }
    }

    // Set the firefighter's incident reference to nullptr (unassigned)
    for (FFirefighterAssignments& FirefighterAssignment : AssignedFirePersonnel)
    {
        if (FirefighterAssignment.Firefighter == Firefighter)
        {
            FirefighterAssignment.Incident = nullptr;
            UE_LOGFMT(LogManager, Display, "{CharacterName} is no longer assigned to an incident."
                , Firefighter->GetCharacterName());
        }
    }
}

/**
 * \brief Unassign Incident from Fire Station
 */
void AGameManager::UnassignIncidentFromFireStation(AWfCalloutActor* IncidentActor, AWfFireStationBase* FireStation)
{
    // Remove the fire station from the incident's assigned stations list
    for (FIncidentAssignments& IncidentAssignment : AssignedIncidents)
    {
        if (IncidentAssignment.Incident == IncidentActor)
        {
            IncidentAssignment.FireStations.Remove(FireStation);
            UE_LOGFMT(LogManager, Display, "Incident #{IncidentNum} is no longer tracking Fire Station #{StationNum}"
                , IncidentActor->GetIncidentNumber(), FireStation->GetFireStationNumber());
        }
    }

    // Remove the incident from the station's active incidents list
    for (FFireStationAssignments& StationAssignment : AssignedFireStations)
    {
        if (StationAssignment.FireStation == FireStation)
        {
            if (StationAssignment.Incidents.Contains(IncidentActor))
            {
                StationAssignment.Incidents.Remove(IncidentActor);
                UE_LOGFMT(LogManager, Display, "{StationNum} is no longer assigned to Incident #{IncidentNum}"
                    , FireStation->GetFireStationNumber(), IncidentActor->GetIncidentNumber());
            }
        }
    }
}

/**
 * \brief Unassign Firefighter from Fire Apparatus
 */
void AGameManager::UnassignFirefighterFromApparatus(AWfFireApparatusBase* FireApparatus, AWfFfCharacterBase* Firefighter)
{
    // Remove the firefighter from the apparatus' assigned firefighters list
    for (FFireApparatusAssignments& ApparatusAssignment : AssignedFireApparatuses)
    {
        if (ApparatusAssignment.FireApparatus == FireApparatus)
        {
            ApparatusAssignment.Firefighters.Remove(Firefighter);
            UE_LOGFMT(LogManager, Display, "{AppIdentity} is no longer tracking {CharacterName}"
                , FireApparatus->GetApparatusIdentity(), Firefighter->GetCharacterName());
        }
    }

    // Set the firefighter's assigned apparatus reference to nullptr (unassigned)
    for (FFirefighterAssignments& FirefighterAssignment : AssignedFirePersonnel)
    {
        if (FirefighterAssignment.Firefighter == Firefighter)
        {
            FirefighterAssignment.FireApparatus = nullptr;
            UE_LOGFMT(LogManager, Display, "{CharacterName} is no longer assigned to {AppIdentity}"
                , Firefighter->GetCharacterName(), FireApparatus->GetApparatusIdentity());
        }
    }
}

// Create the singleton instance
void AGameManager::Initialize()
{
    Instance = this;
}

/**
 * \brief Ensures a hard synchronization between the server and clients
 */
void AGameManager::SyncSimTime()
{
    if (!HasAuthority())
        return;

    CurrentSimTime.TimeSinceStart = CurrentSimTime.SimDateTime - CurrentSimTime.SimTimeStart;
    CurrentSimTime.SyncDateTime   = FDateTime::UtcNow();

    // Replicate
    SynchronizedTime = CurrentSimTime;
}

void AGameManager::BeginPlay()
{
    Super::BeginPlay();

    if (!Instance)
        Initialize();

    // Detect pre-existing actors
	InitializeAssignments(AWfFireStationBase::StaticClass());
	InitializeAssignments(AWfFireApparatusBase::StaticClass());
	InitializeAssignments(AWfFfCharacterBase::StaticClass());
    InitializeAssignments(AWfCalloutActor::StaticClass());

    UE_LOGFMT(LogManager, Display, "{ThisName}({NetMode}): Game Manager Ready!", GetName(), HasAuthority() ? "SRV" : "CLI");

    const FDateTime UtcNow = FDateTime::UtcNow();
    FSimDateTime SdtStruct;
    SdtStruct.SimTimeStart      = UtcNow;
    SdtStruct.SimDateTime       = UtcNow;
    SdtStruct.SyncDateTime      = UtcNow;
    SdtStruct.SimTimeRate       = 0.0f;
    SdtStruct.TimeSinceStart    = FTimespan(0);
    CurrentSimTime = SdtStruct;

    // Initialize the game clock
    if (HasAuthority())
    {
        SyncSimTime();
        GetWorldTimerManager().SetTimer(SimTimerHandle, this, &AGameManager::SyncSimTime, SyncSeconds, true);

        UE_LOGFMT(LogManager, Display
            , "{ThisName}({NetMode}): Game Start Time (UTC) = {SimTime}"
            , GetName(), HasAuthority() ? "SRV" : "CLI", CurrentSimTime.SimTimeStart.ToString());

        UE_LOGFMT(LogManager, Display
            , "{ThisName}({NetMode}): Simulated Date Time (UTC) = {SimTime}"
            , GetName(), HasAuthority() ? "SRV" : "CLI", CurrentSimTime.SimDateTime.ToString());

        UE_LOGFMT(LogManager, Display
            , "{ThisName}({NetMode}): Simulated Time Rate (s) = {SimTime}"
            , GetName(), HasAuthority() ? "SRV" : "CLI", CurrentSimTime.SimTimeRate);

        SetSimulatedTimeRate(1.0f);
    }

    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);
    for (auto& LightActor : DirectionalLights)
    {
        DirectionalLight = Cast<ADirectionalLight>(LightActor);
        return;
    }
}

void AGameManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (Instance)
        Instance = nullptr;
}

void AGameManager::OnRep_AssignedFireApparatuses_Implementation(const TArray<FFireApparatusAssignments>& OldAssignments)
{
    TMap<AWfFireApparatusBase*, FFireApparatusAssignments> OldMap;
    for (const auto& Assignment : OldAssignments)
    {
        OldMap.Add(Assignment.FireApparatus, Assignment);
    }

    for (const auto& NewAssignment : AssignedFireApparatuses)
    {
        if (OldMap.Contains(NewAssignment.FireApparatus))
        {
            const FFireApparatusAssignments& OldAssignment = OldMap[NewAssignment.FireApparatus];
            if (OldAssignment != NewAssignment)
            {
                OnFireApparatusUpdated.Broadcast(OldAssignment, NewAssignment);
            }
            OldMap.Remove(NewAssignment.FireApparatus);
        }
        else
        {
            OnFireApparatusSpawned.Broadcast(NewAssignment.FireApparatus);
        }
    }

    for (const auto& OldAssignment : OldMap)
    {
        OnFireApparatusDestroyed.Broadcast(OldAssignment.Key);
    }
}

void AGameManager::OnRep_AssignedFirePersonnel_Implementation(const TArray<FFirefighterAssignments>& OldAssignments)
{
    TMap<AWfFfCharacterBase*, FFirefighterAssignments> OldMap;
    for (const auto& Assignment : OldAssignments)
    {
        OldMap.Add(Assignment.Firefighter, Assignment);
    }

    for (const auto& NewAssignment : AssignedFirePersonnel)
    {
        if (OldMap.Contains(NewAssignment.Firefighter))
        {
            const FFirefighterAssignments& OldAssignment = OldMap[NewAssignment.Firefighter];
            if (OldAssignment != NewAssignment)
            {
                OnFirefighterUpdated.Broadcast(OldAssignment, NewAssignment);
            }
            OldMap.Remove(NewAssignment.Firefighter);
        }
        else
        {
            OnFirefighterHired.Broadcast(NewAssignment.Firefighter);
        }
    }

    for (const auto& OldAssignment : OldMap)
    {
        OnFirefighterFired.Broadcast(OldAssignment.Key);
    }
}

void AGameManager::OnRep_AssignedFireStations_Implementation(const TArray<FFireStationAssignments>& OldAssignments)
{
    TMap<AWfFireStationBase*, FFireStationAssignments> OldMap;
    for (const auto& Assignment : OldAssignments)
    {
        OldMap.Add(Assignment.FireStation, Assignment);
    }

    for (const auto& NewAssignment : AssignedFireStations)
    {
        if (OldMap.Contains(NewAssignment.FireStation))
        {
            const FFireStationAssignments& OldAssignment = OldMap[NewAssignment.FireStation];
            if (OldAssignment != NewAssignment)
            {
                OnFireStationUpdated.Broadcast(OldAssignment, NewAssignment);
            }
            OldMap.Remove(NewAssignment.FireStation);
        }
        else
        {
            OnFireStationSpawned.Broadcast(NewAssignment.FireStation);
        }
    }

    for (const auto& OldAssignment : OldMap)
    {
        OnFireStationDestroyed.Broadcast(OldAssignment.Key);
    }
}

void AGameManager::OnRep_AssignedIncidents_Implementation(const TArray<FIncidentAssignments>& OldAssignments)
{
    TMap<AWfCalloutActor*, FIncidentAssignments> OldMap;
    for (const auto& Assignment : OldAssignments)
    {
        OldMap.Add(Assignment.Incident, Assignment);
    }

    for (const auto& NewAssignment : AssignedIncidents)
    {
        if (OldMap.Contains(NewAssignment.Incident))
        {
            const FIncidentAssignments& OldAssignment = OldMap[NewAssignment.Incident];
            if (OldAssignment != NewAssignment)
            {
                OnIncidentUpdated.Broadcast(OldAssignment, NewAssignment);
            }
            OldMap.Remove(NewAssignment.Incident);
        }
        else
        {
            OnIncidentStarted.Broadcast(NewAssignment.Incident);
        }
    }

    for (const auto& OldAssignment : OldMap)
    {
        OnIncidentEnded.Broadcast(OldAssignment.Key);
    }
}

void AGameManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CurrentSimTime.SimDateTime += FTimespan::FromSeconds(DeltaTime * CurrentSimTime.SimTimeRate);
    if (HasAuthority())
    {
        AdjustSunLight();
    }
}

void AGameManager::AdjustSunLight()
{
    if (DirectionalLight == nullptr)
        return;

    FTimespan TimeOfDay = CurrentSimTime.SimDateTime.GetTimeOfDay();
    float DayFraction = TimeOfDay.GetTotalSeconds() / 86400.0f; // 86400 seconds in a day

    // Calculate sun angle (azimuth and elevation)
    float SunPitch = 90.0f - (DayFraction * 360.0f); // 90 degrees at noon, -90 degrees at midnight
    float SunYaw = 0.0f; // Simple implementation; could be based on the date for more accuracy

    // Set the light's rotation
    FRotator NewSunRotation(SunPitch, SunYaw, 0.0f);
    DirectionalLight->SetActorRotation(NewSunRotation);
}

/******************************************
 *         NETWORKING & REPLICATION
 */

/**
 * \brief
 * \param OutLifetimeProps
 */
void AGameManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameManager, AssignedIncidents);
	DOREPLIFETIME(AGameManager, AssignedFirePersonnel);
	DOREPLIFETIME(AGameManager, AssignedFireStations);
	DOREPLIFETIME(AGameManager, AssignedIncidents);
}
