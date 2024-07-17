// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WfFireStationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UWfFireStationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * \interface IWfFireStationInterface
 *
 * \brief This is an interface class that defines the functionality for a fire station in a wildfire project.
 * The class contains three interface functions: OnBeginOverlap, OnEndOverlap, and IsEligible.
 */
class PROJECTWILDFIRE_API IWfFireStationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void EventBeginOverlap(AActor* OverlappedActor) = 0;
	virtual void EventEndOverlap(AActor* OverlappedActor) = 0;
	virtual bool IsEligible() { return true; };
};
