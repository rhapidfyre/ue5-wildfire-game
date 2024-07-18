// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/WfVehicleData.h"

FVehicleSeat::FVehicleSeat()
	: SeatNumber(0)
	, SeatOccupant(nullptr)
{

}

FVehicleSeat::FVehicleSeat(const int NewSeatNumber)
{
	SeatNumber = FMath::Abs(NewSeatNumber);
}
