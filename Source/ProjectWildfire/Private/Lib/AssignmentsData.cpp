// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/AssignmentsData.h"

#include "Actors/WfFireStationBase.h"
#include "Characters/WfFfCharacterBase.h"
#include "Lib/WfCalloutData.h"
#include "Vehicles/WfFireApparatusBase.h"


/**
 * \brief
 */
FFirefighterAssignments::FFirefighterAssignments()
	: Firefighter(nullptr), FireApparatus(nullptr), HomeFireStation(nullptr), Incident(nullptr)
{
}

/**
 * \brief
 * \param NewFireFighter
 */
FFirefighterAssignments::FFirefighterAssignments(AWfFfCharacterBase* NewFireFighter)
	: Firefighter(NewFireFighter), FireApparatus(nullptr), HomeFireStation(nullptr), Incident(nullptr)
{
}

/**
 * \brief
 * \param CompareAssignment
 * \return
 */
bool FFirefighterAssignments::operator==(const FFirefighterAssignments& CompareAssignment) const
{
	return (Firefighter == CompareAssignment.Firefighter);
}

/**
 * \brief
 * \param CompareActor
 * \return
 */
bool FFirefighterAssignments::operator==(const AWfFfCharacterBase* CompareActor) const
{
	if (IsValid(Firefighter) && IsValid(CompareActor))
		return Firefighter == CompareActor;
	return false;
}

/**
 * \brief
 */
FFireStationAssignments::FFireStationAssignments()
    : FireStation(nullptr)
{
}

/**
 * \brief
 * \param NewStation
 */
FFireStationAssignments::FFireStationAssignments(AWfFireStationBase* NewStation)
    : FireStation(NewStation)
{
}

/**
 * \brief
 * \param CompareAssignment
 * \return
 */
bool FFireStationAssignments::operator==(const FFireStationAssignments& CompareAssignment) const
{
	return (FireStation == CompareAssignment.FireStation);
}

/**
 * \brief
 * \param CompareActor
 * \return
 */
bool FFireStationAssignments::operator==(const AWfFireStationBase* CompareActor) const
{
	if (IsValid(FireStation) && IsValid(CompareActor))
		return FireStation == CompareActor;
	return false;
}

/**
 * \brief
 */
FFireApparatusAssignments::FFireApparatusAssignments()
    : FireStation(nullptr), Incident(nullptr)
{
}

/**
 * \brief
 * \param NewApparatus
 */
FFireApparatusAssignments::FFireApparatusAssignments(AWfFireApparatusBase* NewApparatus)
    : FireApparatus(NewApparatus), FireStation(nullptr), Incident(nullptr)
{
}

/**
 * \brief
 * \param CompareAssignment
 * \return
 */
bool FFireApparatusAssignments::operator==(const FFireApparatusAssignments& CompareAssignment) const
{
	return (FireApparatus == CompareAssignment.FireApparatus);
}

/**
 * \brief
 * \param CompareActor
 * \return
 */
bool FFireApparatusAssignments::operator==(const AWfFireApparatusBase* CompareActor) const
{
	if (IsValid(FireApparatus) && IsValid(CompareActor))
		return FireApparatus == CompareActor;
	return false;
}

/**
 * \brief
 */
FIncidentAssignments::FIncidentAssignments()
    : Incident(nullptr)
{
}

/**
 * \brief
 * \param NewIncident
 */
FIncidentAssignments::FIncidentAssignments(AWfCalloutActor* NewIncident)
    : Incident(NewIncident)
{
}

/**
 * \brief
 * \param CompareAssignment
 * \return
 */
bool FIncidentAssignments::operator==(const FIncidentAssignments& CompareAssignment) const
{
	return (Incident == CompareAssignment.Incident);
}

/**
 * \brief
 * \param CompareActor
 * \return
 */
bool FIncidentAssignments::operator==(const AWfCalloutActor* CompareActor) const
{
	if (IsValid(Incident) && IsValid(CompareActor))
		return Incident == CompareActor;
	return false;
}
