// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Statics/WfGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Statics/WfGameModeBase.h"

AWfGameStateBase::AWfGameStateBase()
	: GameModeBase(nullptr)
{
}

UWfFirefighterSaveGame* AWfGameStateBase::GetSaveGameFromJobContract(const FJobContractData& JobContract)
{
	if (UGameplayStatics::DoesSaveGameExist(JobContract.ContractId, JobContract.UserIndex))
	{
		USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(JobContract.ContractId, JobContract.UserIndex);
		return Cast<UWfFirefighterSaveGame>(SaveGame);
	}
	return nullptr;
}

void AWfGameStateBase::JobContractOffered(const FJobContractData& JobContract)
{
	TArray TransferList(GameModeBase->GetListOfTransfers());
	if (TransferList.Num() > 0)
	{
		const UWfFirefighterSaveGame* SaveGame = JobContract.SaveReference;
		if (IsValid(SaveGame))
		{
			HiringList.Add(JobContract);
			UE_LOGFMT(LogTemp, Display,
				"JobContractOffered({NetMode}): Added Job Contract #{ContractId} for '{FirstName} {MiddleName} {LastName}' to GameState"
				, HasAuthority()?"Server":"Client", JobContract.ContractId, JobContract.CharacterNameFirst
				, JobContract.CharacterNameMiddle, JobContract.CharacterNameLast);
			if (OnTransferListUpdated.IsBound())
				OnTransferListUpdated.Broadcast(JobContract, true);
		}
	}
}

void AWfGameStateBase::JobContractExpired(const FJobContractData& JobContract)
{
	for (int i = HiringList.Num() - 1; i >= 0; --i)
	{
		if (HiringList.IsValidIndex(i))
		{
			if (HiringList[i].ContractId == JobContract.ContractId)
			{
				HiringList.RemoveAt(i);
				if (OnTransferListUpdated.IsBound())
					OnTransferListUpdated.Broadcast(JobContract, false);
				return;
			}
		}
	}
}

void AWfGameStateBase::JobContractRemove(const FJobContractData& JobContract, bool bDeleteSave)
{
	JobContractRemoveById(JobContract.ContractId, bDeleteSave);
}

void AWfGameStateBase::JobContractRemoveById(const FString& ContractId, bool bDeleteSave)
{
	if (HiringList.Num() > 0)
	{
		for (int i = 0; i < HiringList.Num(); ++i)
		{
			if (HiringList.IsValidIndex(i))
			{
				FJobContractData JobContract = HiringList[i];
				if (JobContract.ContractId == ContractId)
				{
					GameModeBase->JobContractExpired(JobContract, bDeleteSave);
					return;
				}
			}
		}
	}
}

FJobContractData AWfGameStateBase::GetJobContract(const FString& ContractId) const
{
	for (const auto& JobContract : HiringList)
	{
		if (JobContract.ContractId == ContractId)
			return JobContract;
	}
	return {};
}

USaveGame* AWfGameStateBase::CreateNewCharacter(const FGameplayTag& GetRole)
{
	AWfGameModeBase* GameMode = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameMode))
	{
		return GameMode->CreateNewCharacter(GetRole);
	}
	return nullptr;
}

TArray<UWfFirefighterSaveGame*> AWfGameStateBase::GetTransferList() const
{
	const AWfGameModeBase* GameMode = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameMode))
	{
		return GameMode->GetListOfTransfers();
	}
	return {};
}

TArray<FJobContractData>& AWfGameStateBase::GetJobContracts()
{
	return HiringList;
}

void AWfGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GameModeBase = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
		if (IsValid(GameModeBase))
		{
			if (!GameModeBase->OnJobContractOffer.IsAlreadyBound(this, &AWfGameStateBase::JobContractOffered))
			{
				GameModeBase->OnJobContractOffer.AddDynamic(this, &AWfGameStateBase::JobContractOffered);
			}
			if (!GameModeBase->OnJobContractExpired.IsAlreadyBound(this, &AWfGameStateBase::JobContractExpired))
			{
				GameModeBase->OnJobContractExpired.AddDynamic(this, &AWfGameStateBase::JobContractExpired);
			}

			TArray<UWfFirefighterSaveGame*> TransferList = GameModeBase->GetListOfTransfers();
			if ((TransferList.Num() > HiringList.Num()) && TransferList.Num() > 0)
			{
				for (auto& TransferListing : TransferList)
				{
					JobContractOffered(FJobContractData(TransferListing));
				}
			}
		}
	}

	GameModeBase = Cast<AWfGameModeBase>( GetWorld()->GetAuthGameMode() );
	if (IsValid(GameModeBase))
	{
		TArray<UWfFirefighterSaveGame*> TransferList = GetTransferList();
		if (HiringList.Num() < TransferList.Num())
		{
			UE_LOGFMT(LogTemp, Display,
				"GameState HiringList does NOT match the GameMode TransferList ({hNum} != {tNum})"
				, HiringList.Num(), TransferList.Num());
			for (int i = HiringList.Num(); i < TransferList.Num(); ++i)
			{
				JobContractOffered(FJobContractData(TransferList[i]));
			}
		}
		else
		{
			UE_LOGFMT(LogTemp, Display, "HiringList.Num() = {hNum}; TransferList.Num() = {tNum}"
				, HiringList.Num(), TransferList.Num());
		}
	}
	else
	{
		UE_LOGFMT(LogTemp, Warning, "AGameModeBase* Invalid at time of GameState BeginPlay()");
	}
}

/** ************************
 *  REPLICATION / NETWORKING
 */

/**
 * \brief
 * \param OutLifetimeProps
 */
void AWfGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWfGameStateBase, HiringList, COND_OwnerOnly);
}

void AWfGameStateBase::OnRep_HiringList_Implementation(const TArray<FJobContractData>& OldHiringList)
{
	 UE_LOGFMT(LogTemp, Display, "OnRep_HiringList_Implementation()");
	// New job contract added
	for (const auto& NewJobContract : HiringList)
	{
		bool NewContract = true;
		for (const auto& OldJobContract : OldHiringList)
		{
			if (OldJobContract.ContractId == NewJobContract.ContractId)
			{
				NewContract = false;
			}
		}
		if (NewContract)
		{
			UE_LOGFMT(LogTemp, Display, "OnRep_HiringList_Implementation((:)) New Contract: '{fName} {mName} {lName}'",
				NewJobContract.CharacterNameFirst, NewJobContract.CharacterNameMiddle, NewJobContract.CharacterNameLast);
			if (OnTransferListUpdated.IsBound())
				OnTransferListUpdated.Broadcast(NewJobContract, true);
		}
	}

	// Old job contract removed
	for (const auto& OldJobContract : OldHiringList)
	{
		bool ContractLost = true;
		for (const auto& NewJobContract : HiringList)
		{
			if (OldJobContract.ContractId == NewJobContract.ContractId)
			{
				ContractLost = false;
			}
		}
		if (ContractLost)
		{
			UE_LOGFMT(LogTemp, Display, "OnRep_HiringList_Implementation() Contract Expired: '{fName} {mName} {lName}'",
				OldJobContract.CharacterNameFirst, OldJobContract.CharacterNameMiddle, OldJobContract.CharacterNameLast);
			if (OnTransferListUpdated.IsBound())
				OnTransferListUpdated.Broadcast(OldJobContract, false);
		}
	}
}
