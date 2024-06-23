// Fill out your copyright notice in the Description page of Project Settings.

#include "Gas/WfFfAttributeSet.h"

#include "Net/UnrealNetwork.h"


UWfFfAttributeSet::UWfFfAttributeSet()
{
	Health = 100.f;
	Fatigue = 0.f;
	Morale = 100.f;
	Thirst = 0.f;
	Hunger = 0.f;
}

void UWfFfAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWfFfAttributeSet, Health);
	DOREPLIFETIME(UWfFfAttributeSet, Fatigue);
	DOREPLIFETIME(UWfFfAttributeSet, Morale);
	DOREPLIFETIME(UWfFfAttributeSet, Thirst);
	DOREPLIFETIME(UWfFfAttributeSet, Hunger);
}

void UWfFfAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfFfAttributeSet, Health, OldHealth);
}

void UWfFfAttributeSet::OnRep_Fatigue(const FGameplayAttributeData& OldFatigue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfFfAttributeSet, Fatigue, OldFatigue);
}

void UWfFfAttributeSet::OnRep_Morale(const FGameplayAttributeData& OldMorale)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfFfAttributeSet, Morale, OldMorale);
}

void UWfFfAttributeSet::OnRep_Thirst(const FGameplayAttributeData& OldThirst)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfFfAttributeSet, Thirst, OldThirst);
}

void UWfFfAttributeSet::OnRep_Hunger(const FGameplayAttributeData& OldHunger)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfFfAttributeSet, Hunger, OldHunger);
}
