// Fill out your copyright notice in the Description page of Project Settings.


#include "Landscapes/WfRoadSplineBase.h"


AWfRoadSplineBase::AWfRoadSplineBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWfRoadSplineBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfRoadSplineBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
