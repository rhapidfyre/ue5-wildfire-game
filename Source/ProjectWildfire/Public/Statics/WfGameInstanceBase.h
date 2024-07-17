// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "Engine/GameInstance.h"

#include "WfGameInstanceBase.generated.h"


USTRUCT(BlueprintType)
struct PROJECTWILDFIRE_API FMeshOptionsData
{
	GENERATED_BODY()
	FMeshOptionsData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* UsingMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform MeshOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UAnimInstance> UsingAnim;
};

UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API UWfGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()

public:

	UWfGameInstanceBase();

	virtual void Init() override;

	UFUNCTION(BlueprintPure)
	FMeshOptionsData GetMeshOptionsData(const FGameplayTag& NewGender) const;

	UFUNCTION(BlueprintPure)
	float GetMinimumWage() const { return FMath::Clamp(MinimumWage, 0.0f, INT_MAX); }

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinimumWage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FGameplayTag, float> StartingResources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TArray<FMeshOptionsData> MeshesMasculine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	TArray<FMeshOptionsData> MeshesFeminine;

};
