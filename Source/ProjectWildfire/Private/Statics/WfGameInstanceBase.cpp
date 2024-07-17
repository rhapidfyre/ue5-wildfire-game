// Fill out your copyright notice in the Description page of Project Settings.


#include "Statics/WfGameInstanceBase.h"

#include "OnlineSubsystem.h"
#include "Characters/WfCharacterTags.h"


FMeshOptionsData::FMeshOptionsData(): UsingMesh(nullptr)
{
	MeshOffset.SetLocation(FVector(0.0f, 0.0f, -90.0f));
	MeshOffset.SetRotation(FRotator(0.0f, -90.0f, 0.0f).Quaternion());
}

UWfGameInstanceBase::UWfGameInstanceBase(): MinimumWage(0.0f)
{
}

void UWfGameInstanceBase::Init()
{
	Super::Init();

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		UE_LOG(LogTemp, Display, TEXT("OnlineSubsystem found: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found!"));
	}
}

FMeshOptionsData UWfGameInstanceBase::GetMeshOptionsData(const FGameplayTag& NewGender) const
{
	// TODO - Add NonBinary Meshes
	if (NewGender == TAG_Gender_Non_Binary.GetTag())
	{
		TArray<FMeshOptionsData> AllMeshOptions;
		for (const auto& MeshesFem : MeshesFeminine)
			AllMeshOptions.Add(MeshesFem);
		for (const auto& MeshesMasc : MeshesMasculine)
			AllMeshOptions.Add(MeshesMasc);
		if (AllMeshOptions.Num() > 0)
			return AllMeshOptions[FMath::RandRange(0, AllMeshOptions.Num() - 1)];
	}
	if (NewGender == TAG_Gender_Female.GetTag())
	{
		if (MeshesFeminine.Num() > 0)
		{
			return MeshesFeminine[FMath::RandRange(0, MeshesFeminine.Num() - 1)];
		}
	}
	if (MeshesMasculine.Num() > 0)
	{
		return MeshesFeminine[FMath::RandRange(0, MeshesFeminine.Num() - 1)];

	}
	return {};
}
