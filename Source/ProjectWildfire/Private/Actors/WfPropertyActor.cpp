// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WfPropertyActor.h"

#include "Statics/WfGlobalTags.h"


AWfPropertyActor::AWfPropertyActor()
	: PropertyCenter(nullptr), PropertyArea(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	PropertyCenter = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(PropertyCenter);

	PropertyArea = CreateDefaultSubobject<UBoxComponent>("PropertyArea");
	PropertyArea->SetupAttachment(PropertyCenter);

	PropertyTag = TAG_Realty_Residential.GetTag();
}

void AWfPropertyActor::BeginPlay()
{
	Super::BeginPlay();
}

void AWfPropertyActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	DetermineAddress();
}

void AWfPropertyActor::DetermineAddress()
{
	// Get actor world information
	const FVector WorldLocation = GetActorLocation();

	// Determine the street the property is attached to
	// TODO - Detect Street Name
	FString EastWest = "East";
	FString NorthSouth = "South";
	if (WorldLocation.X < 0.0f) EastWest = "West";
	if (WorldLocation.Y < 0.0f) NorthSouth = "North";
	PropertyAddress.StreetName = "First";
	PropertyAddress.StreetType = "Street";

	// Assign Block & House Numbers
	// -Y North +Y South -X West +X East
	PropertyAddress.BlockNumber = FMath::Abs(FMath::RoundToInt(WorldLocation.X / 10000));
	PropertyAddress.HouseNumber = FMath::Abs(FMath::RoundToInt(WorldLocation.X /    10));
}

void AWfPropertyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FStreetAddress AWfPropertyActor::GetStreetAddress() const
{
	return PropertyAddress;
}

FString AWfPropertyActor::GetStreetAddressAsString() const
{
	const FStreetAddress StreetAddress = GetStreetAddress();
	FString HouseNumber = FString::FromInt(StreetAddress.BlockNumber + StreetAddress.HouseNumber);
	FString AppendSuite = "";
	if (StreetAddress.SuiteNumber > 0)
		AppendSuite = " #" + FString::FromInt(StreetAddress.SuiteNumber);
	return HouseNumber + AppendSuite + " " + StreetAddress.StreetName.ToString() + " " + StreetAddress.StreetType.ToString();
}
