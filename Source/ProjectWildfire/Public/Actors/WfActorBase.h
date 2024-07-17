// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Statics/WfGlobalEnums.h"
#include "WfActorBase.generated.h"

class AWfCharacterBase;
class UArrowComponent;

UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfActorBase : public AActor
{
	GENERATED_BODY()

public:

	AWfActorBase();

	// Gets the location the actor should be standing at to begin interacting
	UFUNCTION(BlueprintCallable)
	FVector GetActorMoveToPosition() const;

	// Gets the direction the actor should face once they arrive
	UFUNCTION(BlueprintCallable)
	FRotator GetInteractRotation() const;

	UFUNCTION(BlueprintPure)
	UAnimMontage* GetInteractionAnimation() const;

	UFUNCTION(BlueprintCallable)
	float StartInteraction(AActor* InteractiveActor);

	UFUNCTION(BlueprintCallable)
	void StopInteraction();

protected:

	virtual void BeginPlay() override;

private:

	UFUNCTION()
	void DoInteraction();

	void RemoveEffects(UAbilitySystemComponent* AbilityComponent, TArray<TSubclassOf<UGameplayEffect>> EffectsArray);

	void AddEffects(UAbilitySystemComponent* AbilityComponent, TArray<TSubclassOf<UGameplayEffect>> EffectsArray);

public:

	virtual void Tick(float DeltaTime) override;

	// Seconds to complete interaction with actor. Negative indicates the interaction is instant
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	float SecondsToComplete;

	// How much time between re-use between actors
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	float CooldownTime;

	// Which roles can interact with this actor. If none, the actor will never be used.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	FGameplayTagContainer RolesAllowed;

	// The mesh this actor should use
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	USkeletalMesh* UsingMesh;

	// What attributes are reduced and the value of reduction per completion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TMap<FGameplayAttribute, float> AttributesReduced;

	// What attributes are increased and the value of increase per completion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TMap<FGameplayAttribute, float> AttributesIncreased;

	// Which hours (24h) of the day this item can be used (0=Midnight, 5 = 5AM, 16 = 4PM)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray<int32> HoursAllowed;

	// Which hours (24h) of the day this item can be used (0=Midnight, 5 = 5AM, 16 = 4PM)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	UAnimMontage* InteractionAnim;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Actor Settings")
	UArrowComponent* ArrowComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Actor Settings")
	USceneComponent* RootScene;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsAddedOnStart;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsRemovedOnStart;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsAddedOnSuccess;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsRemovedOnSuccess;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsAddedOnFailure;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsRemovedOnFailure;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsAddedOnStop;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actor Settings")
	TArray< TSubclassOf<UGameplayEffect> > EffectsRemovedOnStop;

protected:

	FTimerHandle TimerInteraction;

	UPROPERTY() AWfCharacterBase* ActorInteracting;

};
