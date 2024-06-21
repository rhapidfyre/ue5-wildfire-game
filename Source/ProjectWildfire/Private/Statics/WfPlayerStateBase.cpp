// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfPlayerStateBase.h"

#include "Logging/StructuredLog.h"
#include "Statics/WfGlobalTags.h"


AWfPlayerStateBase::AWfPlayerStateBase()
	: Resources({})
{
}

void AWfPlayerStateBase::SetResourceValue(const FGameplayTag& ResourceTag, const float NewValue)
{
	if (ResourceTag.IsValid())
	{
		const float OldValue = Resources.Contains(ResourceTag) ? *Resources.Find(ResourceTag) : 0.0f;
		Resources.Add(ResourceTag, NewValue);
		if (OnResourceUpdated.IsBound())
		{
			OnResourceUpdated.Broadcast(ResourceTag, OldValue, NewValue);
			UE_LOGFMT(LogTemp, Display,
				"PlayerState({NetMode}): Resource '{ResourceTag}' Updated ({OldValue} -> {NewValue})",
				ResourceTag.ToString(), OldValue, NewValue);
		}
	}
}

void AWfPlayerStateBase::SetMoney(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetKilowattUsage(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetOxygenReserve(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

void AWfPlayerStateBase::SetWaterStorage(const float NewValue)
{
	SetResourceValue(TAG_Resource_Money.GetTag(), NewValue);
}

float AWfPlayerStateBase::AddMoney(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Money.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddKilowattUsage(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Power.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddOxygenReserve(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Oxygen.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}

float AWfPlayerStateBase::AddWaterStorage(const float AddValue)
{
	const FGameplayTag ResourceTag = TAG_Resource_Water.GetTag();
	const float NewResourceValue = GetResourceValue(ResourceTag) + AddValue;
	SetResourceValue(ResourceTag, NewResourceValue);
	return NewResourceValue;
}


float AWfPlayerStateBase::GetResourceValue(const FGameplayTag& ResourceTag) const
{
	if (ResourceTag.IsValid())
	{
		if (Resources.Contains(ResourceTag))
		{
			const float ResourceValue = Resources[ResourceTag] * 100.0f;
			return FMath::RoundToInt(ResourceValue)/100.0f;
		}
	}
	return 0.0f;
}

FGameplayTagContainer AWfPlayerStateBase::GetAllResourceTags() const
{
	TArray<FGameplayTag> TagArray;
	Resources.GenerateKeyArray(TagArray);
	return FGameplayTagContainer::CreateFromArray(TagArray);
}

float AWfPlayerStateBase::GetMoney() const
{
	return GetResourceValue(TAG_Resource_Money.GetTag());
}

float AWfPlayerStateBase::GetKilowattUsage() const
{
	return GetResourceValue(TAG_Resource_Power.GetTag());
}

float AWfPlayerStateBase::GetOxygenReserve() const
{
	return GetResourceValue(TAG_Resource_Oxygen.GetTag());
}

float AWfPlayerStateBase::GetWaterStorage() const
{
	return GetResourceValue(TAG_Resource_Water.GetTag());
}

void AWfPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();
	SetupInitialResourceValues();
}

void AWfPlayerStateBase::SetupInitialResourceValues()
{
	SetResourceValue(TAG_Resource_Money.GetTag(), 1000.0f);
	SetResourceValue(TAG_Resource_Water.GetTag(), 500.0f);
	SetResourceValue(TAG_Resource_Oxygen.GetTag(),2040.0f);
	SetResourceValue(TAG_Resource_Power.GetTag(), 0.0f);
}
