// Fill out your copyright notice in the Description page of Project Settings.


#include "Statics/WfGlobalData.h"

FStreetAddress::FStreetAddress()
	: BlockNumber(0)
    , HouseNumber(0)
    , SuiteNumber(0)
	, StreetName("Unnamed")
	, StreetType("Street")
{

}

FVoxSounds::FVoxSounds()
	: VoxSound(nullptr)
    , VoxVoice(0)
{
}

FShiftSchedule::FShiftSchedule()
	: DayOfWeek(0),
	  StartTime(FTimespan(6, 30, 0)),
	  Duration(FTimespan(3, 0, 0, 0))
{
}
