// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/WfScheduleComponent.h"

#include "Statics/WfGameModeBase.h"
#include "Statics/WfGlobalData.h"


UWfScheduleComponent::UWfScheduleComponent()
{

}

void UWfScheduleComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UWfScheduleComponent::IsOnDuty() const
{
	return false;
}
