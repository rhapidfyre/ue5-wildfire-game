// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WfFfCharacterBase.h"

#include "Characters/WfCharacterTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WfScheduleComponent.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Saves/WfCharacterSaveGame.h"
#include "Statics/WfGameInstanceBase.h"
#include "Statics/WfGameStateBase.h"
#include "Vehicles/WfFireApparatusBase.h"


AWfFfCharacterBase::AWfFfCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bInQuarters = false;

	AbilityComponent = CreateDefaultSubobject<UWfAbilityComponent>("AbilityComponent");
	AbilityComponent->SetIsReplicated(true);

	ScheduleComponent = CreateDefaultSubobject<UWfScheduleComponent>("ScheduleComponent");
	//ScheduleComponent->SetIsReplicated(false);
}

void AWfFfCharacterBase::SaveCharacter()
{
	Super::SaveCharacter();
}

void AWfFfCharacterBase::SetHourlyRate(float NewHourlyRate)
{
	float MinimumRate = 0.0f;
	const float OldHourlyRate = GetHourlyRate();
	UWfGameInstanceBase* GameInstance = Cast<UWfGameInstanceBase>(GetGameInstance());
	if (IsValid(GameInstance))
		MinimumRate = GameInstance->GetMinimumWage();
	HourlyRate = FMath::Clamp(NewHourlyRate, MinimumRate, INT_MAX);
	if (OnHourlyRateChanged.IsBound())
		OnHourlyRateChanged.Broadcast(OldHourlyRate, GetHourlyRate());
}

FJobContractData AWfFfCharacterBase::GetFirefighterJobContract()
{
	if (!UGameplayStatics::DoesSaveGameExist(ContractId, ContractUserIndex))
	{
		SaveCharacter();
	}
	USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(ContractId, ContractUserIndex);
	if (IsValid(SaveGame))
	{
		FJobContractData JobContract(Cast<UWfFirefighterSaveGame>(SaveGame));
		return JobContract;
	}
	return {};
}

void AWfFfCharacterBase::SetJobContract(const FString& NewContractId, const int32 ContractIndex)
{
	ContractId = NewContractId;
	ContractUserIndex = ContractIndex;
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

bool AWfFfCharacterBase::CanDriveApparatus() const
{
	const FGameplayTag ThisRole = GetCharacterRole();
	if (ThisRole == TAG_Role_Fire_Eng.GetTag())   return true;
	if (ThisRole == TAG_Role_Fire_Chief.GetTag()) return true;
	if (ThisRole == TAG_Role_Fire_Cpt.GetTag())   return true;
	if (ThisRole == TAG_Role_Fire_Arson.GetTag()) return true;
	if (ThisRole == TAG_Role_Fire_Div.GetTag())   return true;
	return false;
}

void AWfFfCharacterBase::BeginPlay()
{
	Super::BeginPlay();
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
	DOREPLIFETIME(AWfFfCharacterBase, ContractId)
	DOREPLIFETIME(AWfFfCharacterBase, ContractUserIndex)
	DOREPLIFETIME_CONDITION(AWfFfCharacterBase, bInQuarters,	COND_OwnerOnly)
	DOREPLIFETIME_CONDITION(AWfFfCharacterBase, HourlyRate,		COND_OwnerOnly)
}
