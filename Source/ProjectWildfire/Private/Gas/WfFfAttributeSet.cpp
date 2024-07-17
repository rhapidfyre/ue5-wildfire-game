// Fill out your copyright notice in the Description page of Project Settings.

#include "Gas/WfFfAttributeSet.h"

#include "Net/UnrealNetwork.h"


UWfFfAttributeSet::UWfFfAttributeSet()
{
}

void UWfFfAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
