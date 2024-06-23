// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WfFfCharacterBase.h"

#include "Characters/WfCharacterTags.h"
#include "Components/CapsuleComponent.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Saves/WfCharacterSaveGame.h"
#include "Statics/WfGameStateBase.h"


AWfFfCharacterBase::AWfFfCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bInQuarters = false;

	AbilityComponent = CreateDefaultSubobject<UWfAbilityComponent>("AbilityComponent");
	AbilityComponent->SetIsReplicated(true);
}

void AWfFfCharacterBase::SetHourlyRate(float NewHourlyRate)
{
	const float OldHourlyRate = GetHourlyRate();
	HourlyRate = NewHourlyRate;
	if (OnHourlyRateChanged.IsBound())
		OnHourlyRateChanged.Broadcast(OldHourlyRate, GetHourlyRate());
}

void AWfFfCharacterBase::EventBeginOverlap(AActor* OverlappedActor)
{
	bInQuarters = true;
	UE_LOGFMT(LogTemp, Display, "{CharacterName} has returned to quarters.", GetName());
}

void AWfFfCharacterBase::EventEndOverlap(AActor* OverlappedActor)
{
	bInQuarters = false;
	UE_LOGFMT(LogTemp, Display, "{CharacterName} has left the station.", GetName());
}

bool AWfFfCharacterBase::IsEligible()
{
	return IWfFireStationInterface::IsEligible();
}

void AWfFfCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilityComponent && HasAuthority())
		AbilityComponent->InitializeAttributes();

}

void AWfFfCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_EngineTraceChannel2, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}


void AWfFfCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWfFfCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AWfFfCharacterBase::LoadCharacter(USaveGame* CharacterSaveGame)
{
	Super::LoadCharacter(CharacterSaveGame);
	const UWfFirefighterSaveGame* FirefighterSave = Cast<UWfFirefighterSaveGame>(CharacterSaveGame);
	if (IsValid(FirefighterSave))
	{
		SetHourlyRate(FirefighterSave->HourlyRate);
		YearsInGrade	= FirefighterSave->YearsInGrade;
		YearsOfService	= FirefighterSave->YearsOfService;
	}
}

void AWfFfCharacterBase::NewCharacter(const FGameplayTag& NewPrimaryRole)
{
	AWfGameStateBase* GameState = Cast<AWfGameStateBase>( GetWorld()->GetGameState() );
	if (IsValid(GameState))
	{
		GameState->CreateNewCharacter(TAG_Role_Fire.GetTag());
	}
}

/**     ************************
 *		REPLICATION & NETWORKING
 */

void AWfFfCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWfFfCharacterBase, YearsOfService)
	DOREPLIFETIME(AWfFfCharacterBase, YearsInGrade)
	DOREPLIFETIME_CONDITION(AWfFfCharacterBase, bInQuarters,	COND_OwnerOnly)
	DOREPLIFETIME_CONDITION(AWfFfCharacterBase, HourlyRate,		COND_OwnerOnly)
}
