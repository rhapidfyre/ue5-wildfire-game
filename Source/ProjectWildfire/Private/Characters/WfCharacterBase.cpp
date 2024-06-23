// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WfCharacterBase.h"

#include "Characters/WfCharacterData.h"
#include "Characters/WfCharacterTags.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Saves/WfCharacterSaveGame.h"
#include "Statics/WfGameStateBase.h"


// Helper function to get all children of a parent FGameplayTag
void GetChildTagsOfParent(const FGameplayTag& ParentTag, TArray<FGameplayTag>& OutChildTags, int& ChildDepth)
{
	if (ChildDepth < 1)
		return;

	ChildDepth--;
	const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	FGameplayTagContainer ChildTags = TagManager.RequestGameplayTagChildren(ParentTag);
	for (const FGameplayTag& ChildTag : ChildTags)
	{
		OutChildTags.Add(ChildTag);

		// Recursively get children of the current child tag
		GetChildTagsOfParent(ChildTag, OutChildTags, ChildDepth);
	}
}

// Sets default values
AWfCharacterBase::AWfCharacterBase()
	: CharacterAge(18)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterRole = TAG_Role_Civilian.GetTag();
}

void AWfCharacterBase::SetCharacterRole(const FGameplayTag& NewRole)
{
	// All possible primary role categories
	TArray<FGameplayTag> ChildTags;
	int ChildDepth = 1;
	GetChildTagsOfParent(TAG_Role.GetTag(), ChildTags, ChildDepth);

	// If new role is any category or subcategory of a primary role, allow it.
	FGameplayTagContainer PossibleTags = FGameplayTagContainer::CreateFromArray(ChildTags);
	if (NewRole.MatchesAny(PossibleTags))
	{
		CharacterRole = NewRole;
		if (OnCharacterRoleSet.IsBound())
			OnCharacterRoleSet.Broadcast(NewRole);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error,
			"{CharName}({NetMode}): Requested Role '{NewRole}' is not a valid primary role type"
			, GetName(), HasAuthority()?"SRV":"CLI", NewRole.ToString());
	}
}

void AWfCharacterBase::SetCharacterGender(const FGameplayTag& NewGender)
{
	if (NewGender.MatchesTag(TAG_Gender.GetTag()))
	{
		CharacterGender = NewGender;
		if (OnCharacterGenderSet.IsBound())
			OnCharacterGenderSet.Broadcast(GetCharacterGender());
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "{CharName}({NetMode}): Invalid Gender Option '{GenderTag}'. Consider adding."
			, GetName(), HasAuthority()?"SRV":"CLI", NewGender.ToString());
	}
}

void AWfCharacterBase::SetCharacterAge(const int NewAge)
{
	if (HasAuthority())
		CharacterAge = FMath::Clamp(NewAge, 18, 99);

	if (OnCharacterAgeSet.IsBound())
		OnCharacterAgeSet.Broadcast(GetCharacterAge());
}

void AWfCharacterBase::LoadCharacter(USaveGame* CharacterSaveGame)
{
	if (!HasAuthority())
		return;

	UWfCharacterSaveGame* CharacterSave = Cast<UWfCharacterSaveGame>(CharacterSaveGame);

	// Saved Character
	if (IsValid(CharacterSave))
	{
		SetCharacterRace(CharacterSave->CharacterRace);
		SetCharacterRole(CharacterSave->CharacterRole);
		SetCharacterAge(CharacterSave->CharacterAge);
		SetCharacterGender(CharacterSave->CharacterGender);

		TArray CharacterNameParts = {
			CharacterSave->CharacterNameFirst,
			CharacterSave->CharacterNameMiddle,
			CharacterSave->CharacterNameLast
		};
		SetCharacterName(CharacterNameParts);
	}

	// New Character
	else
		NewCharacter(TAG_Role_Civilian.GetTag());
}

void AWfCharacterBase::NewCharacter(const FGameplayTag& NewPrimaryRole)
{
	AWfGameStateBase* GameState = Cast<AWfGameStateBase>( GetWorld()->GetGameState() );
	if (IsValid(GameState))
	{
		GameState->CreateNewCharacter(TAG_Role_Civilian.GetTag());
	}
}

// Called when the game starts or when spawned
void AWfCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWfCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWfCharacterBase, NameFirst);
	DOREPLIFETIME(AWfCharacterBase, NameMiddle);
	DOREPLIFETIME(AWfCharacterBase, NameLast);
	DOREPLIFETIME(AWfCharacterBase, CharacterAge);
	DOREPLIFETIME(AWfCharacterBase, EthnicGroup);
	DOREPLIFETIME(AWfCharacterBase, CharacterGender);
	DOREPLIFETIME(AWfCharacterBase, CharacterRole);
}

// Called every frame
void AWfCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AWfCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AWfCharacterBase::EventPrimarySelect()
{
	// Return FALSE if the selection shouldn't work
	return true;
}

bool AWfCharacterBase::EventSecondarySelect()
{
	// Return FALSE if the selection shouldn't work
	return false;
}

void AWfCharacterBase::SetCharacterName(const TArray<FString>& NewCharacterName)
{
	NameFirst  = NewCharacterName[0];
	NameMiddle = NewCharacterName[1];
	NameLast   = NewCharacterName[2];
	if (OnCharacterNameSet.IsBound())
		OnCharacterNameSet.Broadcast(GetCharacterName());
}

void AWfCharacterBase::SetCharacterRace(const FGameplayTag& NewCharacterRace)
{
	EthnicGroup = NewCharacterRace;
	if (OnCharacterRaceSet.IsBound())
		OnCharacterRaceSet.Broadcast(GetCharacterRace());
}

TArray<FString> AWfCharacterBase::GetCharacterNames() const
{
	return {NameFirst, NameMiddle, NameLast};
}

FString AWfCharacterBase::GetCharacterName() const
{
	return FString(NameFirst + " " + NameMiddle + ". " + NameLast);
}


/**     ************************
 *		REPLICATION & NETWORKING
 */



void AWfCharacterBase::OnRep_CharacterRole_Implementation(const FGameplayTag& OldValue)
{
	if (OnCharacterRoleSet.IsBound())
		OnCharacterRoleSet.Broadcast(GetCharacterGender());
}

void AWfCharacterBase::OnRep_CharacterGender_Implementation(const FGameplayTag& OldValue)
{
	if (OnCharacterGenderSet.IsBound())
		OnCharacterGenderSet.Broadcast(GetCharacterGender());
}

void AWfCharacterBase::OnRep_EthnicGroup_Implementation(const FGameplayTag& OldValue)
{
	if (OnCharacterRaceSet.IsBound())
		OnCharacterRaceSet.Broadcast(GetCharacterRace());
}

void AWfCharacterBase::OnRep_CharacterAge_Implementation(const int8& OldValue)
{
	if (OnCharacterAgeSet.IsBound())
		OnCharacterAgeSet.Broadcast(GetCharacterAge());
}

void AWfCharacterBase::OnRep_NameFirst_Implementation(const FString& OldValue)
{
	if (OnCharacterNameSet.IsBound())
		OnCharacterNameSet.Broadcast(GetCharacterName());
}

void AWfCharacterBase::OnRep_NameMiddle_Implementation(const FString& OldValue)
{
	if (OnCharacterNameSet.IsBound())
		OnCharacterNameSet.Broadcast(GetCharacterName());
}

void AWfCharacterBase::OnRep_NameLast_Implementation(const FString& OldValue)
{
	if (OnCharacterNameSet.IsBound())
		OnCharacterNameSet.Broadcast(GetCharacterName());
}
