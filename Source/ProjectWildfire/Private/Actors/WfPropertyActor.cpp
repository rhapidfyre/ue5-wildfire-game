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
	PropertyAddress.StreetName = "Untitled";
	PropertyAddress.StreetType = "Street";

	// Assign Block & House Numbers
	PropertyAddress.BlockNumber = FMath::RoundToInt(WorldLocation.X / 10000);
	PropertyAddress.HouseNumber = FMath::RoundToInt(WorldLocation.X /   100);
}

void AWfPropertyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
