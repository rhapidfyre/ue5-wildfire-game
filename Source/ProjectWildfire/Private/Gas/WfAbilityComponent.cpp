// Fill out your copyright notice in the Description page of Project Settings.

#include "Gas/WfAbilityComponent.h"
#include "Gas/WfFfAttributeSet.h"
#include "Logging/StructuredLog.h"

UWfAbilityComponent::UWfAbilityComponent()
	: StatReduction(0.1f)
	, ReductionFatigue(0.0f)
	, ReductionThirst(0.07f)
	, ReductionHunger(0.0f)
	, AttributeTimeRate(0.5f)
{
	SetIsReplicatedByDefault(true);
}

void UWfAbilityComponent::TimerTick()
{
	if (IsValid(CharacterAttributes))
	{
		AddHunger(StatReduction + ReductionHunger);
		AddThirst(StatReduction + ReductionThirst);
	}
}

void UWfAbilityComponent::SetNewTimerRate(const float NewTimeRate)
{
	if (NewTimeRate != AttributeTimeRate || !GetWorld()->GetTimerManager().IsTimerActive(AttributeTimer))
	{
		AttributeTimeRate = NewTimeRate;
		if (AttributeTimer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(AttributeTimer);
		}

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &UWfAbilityComponent::TimerTick);
		GetWorld()->GetTimerManager().SetTimer(
			AttributeTimer, TimerDelegate, AttributeTimeRate, true);
	}
}

float UWfAbilityComponent::SetHunger(float NewValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetHungerAttribute();
		SetNewAttributeValue(GameAttribute, NewValue);
		return CharacterAttributes->GetHunger();
	}
	return false;
}

float UWfAbilityComponent::ModifyHunger(float PercentChange)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetHungerAttribute();
		ModifyAttributePercent(GameAttribute, PercentChange);
		return CharacterAttributes->GetHunger();
	}
	return false;
}

float UWfAbilityComponent::AddHunger(float ModValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetHungerAttribute();
		const float OldValue = CharacterAttributes->GetHunger();
		SetNewAttributeValue(GameAttribute, OldValue + ModValue);
		return CharacterAttributes->GetHunger();
	}
	return false;
}

float UWfAbilityComponent::RemoveHunger(float ModValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetHungerAttribute();
		const float OldValue = CharacterAttributes->GetHunger();
		SetNewAttributeValue(GameAttribute, OldValue - ModValue);
		return CharacterAttributes->GetHunger();
	}
	return false;
}

float UWfAbilityComponent::SetThirst(float NewValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetThirstAttribute();
		SetNewAttributeValue(GameAttribute, NewValue);
		return CharacterAttributes->GetThirst();
	}
	return false;
}

float UWfAbilityComponent::ModifyThirst(float PercentChange)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetThirstAttribute();
		ModifyAttributePercent(GameAttribute, PercentChange);
		return CharacterAttributes->GetThirst();
	}
	return false;
}

float UWfAbilityComponent::AddThirst(float ModValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetThirstAttribute();
		const float OldValue = CharacterAttributes->GetThirst();
		SetNewAttributeValue(GameAttribute, OldValue + ModValue);
		return CharacterAttributes->GetThirst();
	}
	return false;
}

float UWfAbilityComponent::RemoveThirst(float ModValue)
{
	if (IsValid(CharacterAttributes))
	{
		const FGameplayAttribute GameAttribute = CharacterAttributes->GetThirstAttribute();
		const float OldValue = CharacterAttributes->GetThirst();
		SetNewAttributeValue(GameAttribute, OldValue - ModValue);
		return CharacterAttributes->GetThirst();
	}
	return false;
}

void UWfAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWfAbilityComponent::InitializeAttributes(UWfAttributeSet* AttributeSet)
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(AttributeSet))
		{
			CharacterAttributes = AttributeSet;
			CharacterAttributes->OnAttributeUpdated.AddDynamic(this, &UWfAbilityComponent::AttributeUpdated);
			SetNewTimerRate(AttributeTimeRate);
		}
	}
}

void UWfAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(UWfAbilityComponent, ActiveGameplayEffects);
}

void UWfAbilityComponent::SetNewAttributeValue(const FGameplayAttribute& GameAttribute, const float NewValue)
{
	if (!GameAttribute.IsValid())
		return;

	CharacterAttributes->SetAttributeValue(
		GameAttribute, FMath::Clamp(NewValue, 0.0f, 100.0f));
}

void UWfAbilityComponent::ModifyAttributePercent(const FGameplayAttribute& GameAttribute, const float PercentChange)
{
	if (!GameAttribute.IsValid())
		return;

	CharacterAttributes->ModifyAttributePercent(
		GameAttribute, FMath::Clamp(PercentChange, -1.0f, 1.0f));
}

void UWfAbilityComponent::AttributeUpdated(const FGameplayAttribute& GameAttribute, const float OldValue,
                                           const float NewValue)
{
	if (OnAttributeChanged.IsBound())
		OnAttributeChanged.Broadcast(GameAttribute, OldValue, NewValue);
}
