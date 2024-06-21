// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WfCharacterBase.h"

#include "Characters/WfCharacterData.h"
#include "Characters/WfCharacterTags.h"

struct FEthnicGroup
{
	FGameplayTag GroupName;
	double Chance;
};

// Sets default values
AWfCharacterBase::AWfCharacterBase()
	: FirstNamesTable(nullptr)
	, LastNamesTable(nullptr)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWfCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterAge	= FMath::RandRange(19,64);
	if (OnCharacterAgeSet.IsBound())
	{
		OnCharacterAgeSet.Broadcast(GetCharacterAge());
	}

	CharacterGender = (FMath::RandRange(0.0f, 1.0f) > 0.15 ? TAG_Gender_Male : TAG_Gender_Female).GetTag();
	if (FMath::RandRange(0.0f,1.0f) < 0.005)
		{ CharacterGender = TAG_Gender_Non_Binary.GetTag(); }

	if (OnCharacterGenderSet.IsBound())
	{
		OnCharacterGenderSet.Broadcast(GetCharacterGender());
	}

	CharacterRole = TAG_Role_Firefighter.GetTag();
	if (OnCharacterRoleSet.IsBound())
	{
		OnCharacterRoleSet.Broadcast(GetCharacterRole());
	}

	GenerateRandomRace();
	GenerateRandomName();
}

FGameplayTag AWfCharacterBase::PickRandomEthnicGroup()
{
	TArray<FEthnicGroup> EthnicGroups = {
		{TAG_Ethnicity_White.GetTag(), 0.616},
		{TAG_Ethnicity_Black.GetTag(), 0.124},
		{TAG_Ethnicity_Hispanic.GetTag(), 0.187},
		{TAG_Ethnicity_Asian.GetTag(), 0.06},
		{TAG_Ethnicity_NativeAmerican.GetTag(), 0.029},
		{TAG_Ethnicity_PacificIslander.GetTag(), 0.002}
	};

	double RandValue = FMath::FRand();
	double CumulativeChance = 0.0;

	for (const auto& Group : EthnicGroups)
	{
		CumulativeChance += Group.Chance;
		if (RandValue < CumulativeChance)
		{
			return Group.GroupName;
		}
	}

	return EthnicGroups[0].GroupName;
}

FGameplayTag AWfCharacterBase::PickRandomFatherEthnicGroup(const FGameplayTag& MotherEthnicGroup)
{
	double InterracialChance = 0.151;
	double RandValue = FMath::FRand();

	if (RandValue < InterracialChance)
	{
		return PickRandomEthnicGroup();
	}
	return MotherEthnicGroup;
}

void AWfCharacterBase::DetermineMixedRaceOutcome(
	const FGameplayTag& MotherEthnicGroup, const FGameplayTag& FatherEthnicGroup)
{
	if (MotherEthnicGroup != FatherEthnicGroup)
	{
		EthnicGroup = FMath::RandRange(0,1) == 0 ? MotherEthnicGroup : FatherEthnicGroup;
		return;
	}
	EthnicGroup = MotherEthnicGroup;
	if (OnCharacterRaceSet.IsBound())
	{
		OnCharacterRaceSet.Broadcast(GetEthnicity());
	}
}

void AWfCharacterBase::GenerateRandomName()
{
	const FGameplayTag MyGender = GetCharacterGender();
	const FGameplayTag MyEthnic = GetEthnicity();

	// Ensure data tables are valid
	if (!FirstNamesTable || !LastNamesTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Name DataTables are not set!"));
		NameFirst	= MyGender == TAG_Gender_Male.GetTag() ? "John" : "Jane";
		NameMiddle	= "Q";
		NameLast	= "Public";
		return;
	}

	// Get random first name
	const FString FirstNameContext = "";
	TArray<FWfNamesStruct*> FirstNameRows;
	FirstNamesTable->GetAllRows<FWfNamesStruct>(FirstNameContext, FirstNameRows);
	TArray<FWfNamesStruct*> FirstNames;
	for (const auto& FirstNameRow : FirstNameRows)
	{
		if (FirstNameRow->EthnicGroups.HasTag(MyEthnic))
		{
			if (FirstNameRow->bFeminine && MyGender != TAG_Gender_Male.GetTag())
				{ FirstNames.Add(FirstNameRow); }
			else if (FirstNameRow->bMasculine && MyGender != TAG_Gender_Female.GetTag())
				{ FirstNames.Add(FirstNameRow); }
		}
	}
	int32 FirstNameIndex = FMath::RandRange(0, FirstNames.Num() - 1);
	NameFirst = FirstNames[FirstNameIndex]->NameValue;

	// Get random last name
	const FString LastNameContext;
	TArray<FWfNamesStruct*> LastNameRows;
	LastNamesTable->GetAllRows<FWfNamesStruct>(LastNameContext, LastNameRows);
	TArray<FWfNamesStruct*> LastNames;
	for (const auto& LastNameRow : LastNameRows)
	{
		if (LastNameRow->EthnicGroups.HasTag(MyEthnic))
		{
			if (LastNameRow->bFeminine && MyGender != TAG_Gender_Male.GetTag())
				{ LastNames.Add(LastNameRow); }
			else if (LastNameRow->bMasculine && MyGender != TAG_Gender_Female.GetTag())
				{ LastNames.Add(LastNameRow); }
		}
	}
	int32 LastNameIndex = FMath::RandRange(0, LastNames.Num() - 1);
	NameLast = LastNames[LastNameIndex]->NameValue;

	// Get random middle initial
	char MiddleInitialChar = static_cast<char>(FMath::RandRange(65, 90));
	NameMiddle = FString(1, &MiddleInitialChar);

	if (OnCharacterNameSet.IsBound())
	{
		OnCharacterNameSet.Broadcast(GetCharacterName());
	}

}

void AWfCharacterBase::GenerateRandomRace()
{
	// Generate mother's race
	FGameplayTag MotherRace = PickRandomEthnicGroup();

	// Generate father's race
	FGameplayTag FatherRace = PickRandomFatherEthnicGroup(MotherRace);

	// Determine mixed race outcome
	DetermineMixedRaceOutcome(MotherRace, FatherRace);

	// Output the result (for testing purposes)
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Mother's Race: %s"), *MotherRace.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Father's Race: %s"), *FatherRace.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Character's Race: %s"), *GetEthnicity().ToString()));
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

TArray<FString> AWfCharacterBase::GetCharacterNames() const
{
	return {NameFirst, NameMiddle, NameLast};
}

FString AWfCharacterBase::GetCharacterName() const
{
	return FString(NameFirst + " " + NameMiddle + ". " + NameLast);
}
