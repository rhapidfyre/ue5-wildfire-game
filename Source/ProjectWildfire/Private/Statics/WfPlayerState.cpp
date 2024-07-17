// Fill out your copyright notice in the Description page of Project Settings.


#include "Statics/WfPlayerState.h"

#include "OnlineSubsystem.h"
#include "Characters/WfCharacterTags.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Saves/WfPlayerSave.h"
#include "Statics/WfGameModeBase.h"

FString GetSteamId()
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
	{
		const IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			const TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
			if (UserId.IsValid())
			{
				// Cast to the Steam-specific ID type
				FString SteamId = UserId->ToString();
				return SteamId;
			}
		}
	}
	return FString();
}

AWfPlayerState::AWfPlayerState()
	: SaveSlotName("SaveSlot")
	, SaveUserIndex(0)
{

}

void AWfPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SaveSlotName  = GetSteamId();
		SaveUserIndex = 0;

		// Reload Saved Game
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
		{
			UE_LOGFMT(LogTemp, Display, "Reloading Existing Player Save ({SaveSlotName} {uIndex}", SaveSlotName, SaveUserIndex);
			UWfPlayerSave* PlayerSave = Cast<UWfPlayerSave>( UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex) );
			if (IsValid(PlayerSave))
			{
				for (const auto& SavedPersonnelData : PlayerSave->SavedPersonnel)
				{
					FString FailureContext;
					AcceptJobContract(SavedPersonnelData, FailureContext);
				}
			}
			UE_LOGFMT(LogTemp, Display, "Finished Reloading Player Save");
		}
		else
		{
			UE_LOGFMT(LogTemp, Display, "No Player Save Found - Starting NEW Game");

			// Start with a full team
			TArray<FGameplayTag> GeneratePersonnel;
			GeneratePersonnel.Add(TAG_Role_Fire_Cpt.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Cpt.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Cpt.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Eng.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Eng.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Eng.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_One.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_One.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_One.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_One.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Two.GetTag());
			GeneratePersonnel.Add(TAG_Role_Fire_Two.GetTag());

			for (auto& GenerateTag : GeneratePersonnel)
			{
				AWfGameModeBase* GameModeReference = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
				if (IsValid(GameModeReference))
				{
					UWfSaveGame* NewSave = GameModeReference->CreateNewCharacter(GenerateTag);
					FJobContractData NewContract(Cast<UWfFirefighterSaveGame>(NewSave));
					FString FailureContext;
					AcceptJobContract(NewContract, FailureContext);
				}
			}

			UWfPlayerSave* NewPlayerSave = Cast<UWfPlayerSave>
				(UGameplayStatics::CreateSaveGameObject(UWfPlayerSave::StaticClass()));

			if (IsValid(NewPlayerSave))
			{
				for (const auto& PersonnelData : AllPersonnel)
				{
					NewPlayerSave->SavedPersonnel.Add(PersonnelData->GetFirefighterJobContract());
				}
				UGameplayStatics::SaveGameToSlot(NewPlayerSave, SaveSlotName, SaveUserIndex);
			}

			if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
				UE_LOGFMT(LogTemp, Display, "New Save Game Created: '{SaveSlot} {uIndex}'", SaveSlotName, SaveUserIndex);
			else
				UE_LOGFMT(LogTemp, Error, "New Save Game Creation FAILED: '{SaveSlot} {uIndex}'", SaveSlotName, SaveUserIndex);
		}

	}
}
