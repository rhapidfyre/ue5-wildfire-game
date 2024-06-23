// Fill out your copyright notice in the Description page of Project Settings.

#include "Gas/WfAbilityComponent.h"
#include "Gas/WfFfAttributeSet.h"
#include "Net/UnrealNetwork.h"

UWfAbilityComponent::UWfAbilityComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWfAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		InitializeAttributes();
	}
}

void UWfAbilityComponent::InitializeAttributes()
{
	if (GetOwner()->HasAuthority())
	{
		UWfFfAttributeSet* AttributeSet = NewObject<UWfFfAttributeSet>(this, TEXT("FfAttributeSet"));
		AddAttributeSetSubobject(AttributeSet);
	}
}

void UWfAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(UWfAbilityComponent, ActiveGameplayEffects);
}
