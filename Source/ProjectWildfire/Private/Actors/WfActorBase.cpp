// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WfActorBase.h"

#include "Characters/WfCharacterBase.h"
#include "Characters/WfCharacterTags.h"
#include "Components/ArrowComponent.h"
#include "Gas/WfAbilityComponent.h"
#include "Logging/StructuredLog.h"


AWfActorBase::AWfActorBase()
	: SecondsToComplete(0)
	, CooldownTime(3.0f)
	, UsingMesh(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	if (RolesAllowed.IsEmpty())
	{
		RolesAllowed.AddTag(TAG_Role_Fire);
	}

	RootScene = CreateDefaultSubobject<USceneComponent>("RootScene");
	SetRootComponent(RootScene);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
	ArrowComponent->SetupAttachment(RootScene);
	ArrowComponent->ArrowSize	= 0.5f;
	ArrowComponent->ArrowColor	= FColor::White;
}

FVector AWfActorBase::GetActorMoveToPosition() const
{
	return IsValid(ArrowComponent) ? ArrowComponent->GetComponentLocation() : FVector(0.0f);
}

FRotator AWfActorBase::GetInteractRotation() const
{
	return IsValid(ArrowComponent) ? ArrowComponent->GetComponentRotation() : FRotator(0.0f);
}

UAnimMontage* AWfActorBase::GetInteractionAnimation() const
{
	if (IsValid(InteractionAnim))
	{
		return InteractionAnim;
	}
	return nullptr;
}

/**
 * \brief Begins an interaction transaction between this actor and the activator
 * \param InteractiveActor The actor who is interacting with this actor
 * \return Number of seconds required to activate. Returns negative on failure. Zero means instant.
 */
float AWfActorBase::StartInteraction(AActor* InteractiveActor)
{
	if (!IsValid(InteractiveActor))
	{
		UE_LOGFMT(LogTemp, Error, "Interacting Actor is Invalid");
		return -1.0f;
	}

	if (!TimerInteraction.IsValid())
	{
		ActorInteracting = Cast<AWfCharacterBase>(InteractiveActor);
		UWfAbilityComponent* AbilityComponent = ActorInteracting->AbilityComponent;
		AddEffects(AbilityComponent, EffectsAddedOnStart);
		RemoveEffects(AbilityComponent, EffectsRemovedOnStart);

		if (SecondsToComplete > 0.0f)
		{
			FTimerDelegate ActionDelegate;
			ActionDelegate.BindUObject(this, &AWfActorBase::DoInteraction);
			UE_LOGFMT(LogTemp, Display, "{ThisActor}({NetMode}): Actor '{InteractionActor}' has started interaction."
				, GetName(), HasAuthority() ? "SRV" : "CLI", ActorInteracting->GetName());
			GetWorld()->GetTimerManager().SetTimer(TimerInteraction, ActionDelegate, SecondsToComplete, false);
		}
		else
		{
			UE_LOGFMT(LogTemp, Display, "{ThisActor}({NetMode}): Actor '{InteractionActor}' is interacting."
				, GetName(), HasAuthority() ? "SRV" : "CLI", ActorInteracting->GetName());
			DoInteraction();
		}
		return SecondsToComplete;
	}
	return -1.0f;
}

void AWfActorBase::StopInteraction()
{
	if (!IsValid(ActorInteracting))
	{
		ActorInteracting = nullptr;
		if (TimerInteraction.IsValid())
			TimerInteraction.Invalidate();
		return;
	}

	UWfAbilityComponent* AbilityComponent = ActorInteracting->AbilityComponent;
	if (TimerInteraction.IsValid())
	{
		AddEffects(AbilityComponent, EffectsAddedOnFailure);
		RemoveEffects(AbilityComponent, EffectsRemovedOnFailure);
		TimerInteraction.Invalidate();
	}

	UE_LOGFMT(LogTemp, Display, "{ThisActor}({NetMode}): Actor '{InteractionActor}' has stopped interacting."
	, GetName(), HasAuthority() ? "SRV" : "CLI", ActorInteracting->GetName());

	AddEffects(AbilityComponent, EffectsAddedOnStop);
	RemoveEffects(AbilityComponent, EffectsRemovedOnStop);

	ActorInteracting = nullptr;
}

void AWfActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfActorBase::DoInteraction()
{
	if (!IsValid(ActorInteracting))
	{
		UE_LOGFMT(LogTemp, Error, "{ThisActor}({NetMode}): ActorInteracting was INVALID."
			, GetName(), HasAuthority() ? "SRV" : "CLI");
		return;
	}

	if (!IsValid(ActorInteracting->AbilityComponent))
	{
		UE_LOGFMT(LogTemp, Error, "{ThisActor}({NetMode}): Actor '{InteractionActor}' does not have a valid Ability Component."
			, GetName(), HasAuthority() ? "SRV" : "CLI", ActorInteracting->GetName());
		return;
	}

	UWfAbilityComponent* AbilityComponent = ActorInteracting->AbilityComponent;
	AddEffects(AbilityComponent, EffectsAddedOnStart);
	RemoveEffects(AbilityComponent, EffectsRemovedOnStart);
}

void AWfActorBase::RemoveEffects(UAbilitySystemComponent* AbilityComponent, TArray<TSubclassOf<UGameplayEffect>> EffectsArray)
{
	if (!IsValid(ActorInteracting))
		return;
	if (!IsValid(ActorInteracting->AbilityComponent))
		return;

	const FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
	for (const auto& GameplayEffect : EffectsArray)
	{
		const UGameplayEffect* EffectObject = GameplayEffect->GetDefaultObject<UGameplayEffect>();
		if (IsValid(EffectObject))
		{
			FGameplayEffectQuery EffectQuery;
			EffectQuery.EffectDefinition = GameplayEffect;
			UE_LOGFMT(LogTemp, Display, "{ThisActor}({NetMode}): Removing Effect '{AppliedEffect}' to Actor '{InteractionActor}'"
				, GetName(), HasAuthority() ? "SRV" : "CLI", GameplayEffect->GetName(), ActorInteracting->GetName());
			AbilityComponent->RemoveActiveEffects(EffectQuery);
		}
	}
}

void AWfActorBase::AddEffects(UAbilitySystemComponent* AbilityComponent, TArray<TSubclassOf<UGameplayEffect>> EffectsArray)
{
	if (!IsValid(ActorInteracting))
		return;
	if (!IsValid(ActorInteracting->AbilityComponent))
		return;

	const FGameplayEffectContextHandle EffectContext = AbilityComponent->MakeEffectContext();
	for (const auto& GameplayEffect : EffectsArray)
	{
		const UGameplayEffect* EffectObject = GameplayEffect->GetDefaultObject<UGameplayEffect>();
		if (IsValid(EffectObject))
		{
			AbilityComponent->ApplyGameplayEffectToSelf(EffectObject, 1, EffectContext);
			UE_LOGFMT(LogTemp, Display, "{ThisActor}({NetMode}): Applied Effect '{AppliedEffect}' to Actor '{InteractionActor}'"
				, GetName(), HasAuthority() ? "SRV" : "CLI", GameplayEffect->GetName(), ActorInteracting->GetName());
		}
	}
}

void AWfActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
