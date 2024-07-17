// Fill out your copyright notice in the Description page of Project Settings.


#include "Gas/WfAttributeSet.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

UWfAttributeSet::UWfAttributeSet()
	: Health(100.0f)
	, Fatigue(0.0f)
	, Morale(50.0f)
	, Thirst(0.0f)
	, Hunger(0.0f)
{
	Health.SetBaseValue(100.f);
	Fatigue.SetBaseValue(100.f);
	Morale.SetBaseValue(100.f);
	Thirst.SetBaseValue(100.f);
	Hunger.SetBaseValue(100.f);
}

/**
 * \brief Sets the requested attribute to the specified value
 * \param GameAttribute The attribute to be modified
 * \param NewValue The value the attribute should be modified to (exact)
 * \return True on success, false on failure
 */
bool UWfAttributeSet::SetAttributeValue(const FGameplayAttribute& GameAttribute, float NewValue)
{
	if (!GameAttribute.IsValid())
		return false;

	const float OldValue = GetAttributeValue(GameAttribute);
	const float FinalValue = FMath::Clamp(NewValue, 0.0f, 100.0f);

	if (OldValue == FinalValue)
		return false;

	if (GameAttribute == GetHealthAttribute())	SetHealth(FinalValue);
	if (GameAttribute == GetMoraleAttribute())  SetMorale(FinalValue);
	if (GameAttribute == GetFatigueAttribute()) SetFatigue(FinalValue);
	if (GameAttribute == GetHungerAttribute())  SetHunger(FinalValue);
	if (GameAttribute == GetThirstAttribute())  SetThirst(FinalValue);

	if (OnAttributeUpdated.IsBound())
		OnAttributeUpdated.Broadcast(GameAttribute, OldValue, GetAttributeValue(GameAttribute));

	return true;
}

float UWfAttributeSet::ModifyAttribute(const FGameplayAttribute& GameAttribute, float AddValue)
{
	if (!GameAttribute.IsValid())
		return false;
	return SetAttributeValue(GameAttribute, GetAttributeValue(GameAttribute) + AddValue);
}

float UWfAttributeSet::ModifyAttributePercent(const FGameplayAttribute& GameAttribute, float PercentChange)
{
	if (!GameAttribute.IsValid())
		return false;
	const float OldValue = GetAttributeValue(GameAttribute);
	const float ValueChange = OldValue * PercentChange;
	return SetAttributeValue(GameAttribute,
		FMath::Clamp(OldValue + ValueChange, 0.0f, 100.0f));
}

/**
 * \brief Gets the float value of the attribute requested
 * \param GameAttribute The attribute to retrieve
 * \return Returns a negative value upon failure
 */
float UWfAttributeSet::GetAttributeValue(const FGameplayAttribute& GameAttribute) const
{
	if (!GameAttribute.IsValid())
		return -100.0f;
	if (GameAttribute == GetHealthAttribute())	{ return GetHealth(); }
	if (GameAttribute == GetMoraleAttribute())  { return GetMorale(); }
	if (GameAttribute == GetFatigueAttribute()) { return GetFatigue(); }
	if (GameAttribute == GetHungerAttribute())  { return GetHunger(); }
	if (GameAttribute == GetThirstAttribute())  { return GetThirst(); }
	return -100.0f;
}


void UWfAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWfAttributeSet, Health);
	DOREPLIFETIME(UWfAttributeSet, Fatigue);
	DOREPLIFETIME(UWfAttributeSet, Morale);
	DOREPLIFETIME(UWfAttributeSet, Thirst);
	DOREPLIFETIME(UWfAttributeSet, Hunger);
}

void UWfAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfAttributeSet, Health, OldHealth);
	if (OnHealthChanged.IsBound())
		OnHealthChanged.Broadcast(OldHealth.GetCurrentValue(), Health.GetCurrentValue());
}

void UWfAttributeSet::OnRep_Fatigue(const FGameplayAttributeData& OldFatigue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfAttributeSet, Fatigue, OldFatigue);
	if (OnHealthChanged.IsBound())
		OnHealthChanged.Broadcast(OldFatigue.GetCurrentValue(), Fatigue.GetCurrentValue());
}

void UWfAttributeSet::OnRep_Morale(const FGameplayAttributeData& OldMorale)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfAttributeSet, Morale, OldMorale);
	if (OnHealthChanged.IsBound())
		OnHealthChanged.Broadcast(OldMorale.GetCurrentValue(), Morale.GetCurrentValue());
}

void UWfAttributeSet::OnRep_Thirst(const FGameplayAttributeData& OldThirst)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfAttributeSet, Thirst, OldThirst);
	if (OnHealthChanged.IsBound())
		OnHealthChanged.Broadcast(OldThirst.GetCurrentValue(), Thirst.GetCurrentValue());
}

void UWfAttributeSet::OnRep_Hunger(const FGameplayAttributeData& OldHunger)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWfAttributeSet, Hunger, OldHunger);
	if (OnHealthChanged.IsBound())
		OnHealthChanged.Broadcast(OldHunger.GetCurrentValue(), Hunger.GetCurrentValue());
}
