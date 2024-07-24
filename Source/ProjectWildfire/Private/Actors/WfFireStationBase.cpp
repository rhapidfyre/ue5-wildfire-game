// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WfFireStationBase.h"

#include "Controllers/WfPlayerControllerBase.h"
#include "Interfaces/WfFireStationInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "Statics/WfGameModeBase.h"


UParkingSpotComponent::UParkingSpotComponent()
	: AssignedVehicle(nullptr)
{
}

// Sets default values
AWfFireStationBase::AWfFireStationBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// The boundary box for determining if something is within the Fire Station area
	BoundaryBox = CreateDefaultSubobject<UBoxComponent>("BoundaryBox");
	BoundaryBox->SetupAttachment(RootComponent);
	BoundaryBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoundaryBox->SetCollisionObjectType(ECC_EngineTraceChannel2);
	BoundaryBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // All Pawns
	BoundaryBox->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap); // WfEntities
	BoundaryBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	BoundaryBox->SetBoxExtent(FVector(100.0f), false);

	BoundaryBox->OnComponentBeginOverlap.AddDynamic(this, &AWfFireStationBase::OnOverlapBegin);
	BoundaryBox->OnComponentEndOverlap.AddDynamic(this, &AWfFireStationBase::OnOverlapEnd);
	SetActorEnableCollision(true);

	SpawnPoint = CreateDefaultSubobject<USceneComponent>("SpawnPoint");
	SpawnPoint->SetupAttachment(BoundaryBox);

	// Sets up one default parking spot by default
	ParkingSpots.Empty();
	UParkingSpotComponent* DefaultParkingSpot = CreateDefaultSubobject<UParkingSpotComponent>("ParkingSpot_01");
	DefaultParkingSpot->SetupAttachment(GetRootComponent());
	DefaultParkingSpot->SetWorldLocation(SpawnPoint->GetComponentLocation());
	DefaultParkingSpot->SetWorldRotation(SpawnPoint->GetComponentRotation());
	ParkingSpots.Add(DefaultParkingSpot);
}

UParkingSpotComponent* AWfFireStationBase::GetParkingSpot(const int ParkingSpotNumber) const
{
	if (ParkingSpots.IsValidIndex(ParkingSpotNumber))
	{
		return ParkingSpots[ParkingSpotNumber];
	}
	return nullptr;
}

// Called when the game starts or when spawned
void AWfFireStationBase::BeginPlay()
{
	Super::BeginPlay();

	// Get the player controller
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (AWfPlayerControllerBase* WfPlayerController = Cast<AWfPlayerControllerBase>(PlayerController))
	{
		// Bind to the OnPlayerControllerReady delegate
		WfPlayerController->OnActorHitBySelection.AddDynamic(this, &AWfFireStationBase::EventMouseSelect);
	}
}

void AWfFireStationBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BoundaryBox->UpdateOverlaps(); // Update overlaps when the size changes

	if (FireStationNumber < 1)
	{
		if (AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode())
		{
			if (AWfGameModeBase* WfGameMode = Cast<AWfGameModeBase>(GameModeBase))
			{
				FireStationNumber = WfGameMode->GetNextFireStationNumber();
			}
		}
	}
}

void AWfFireStationBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWfFireStationBase, FireStationNumber);
}

// Called every frame
void AWfFireStationBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * \brief Fired when a select event hit an actor that did not implement the clickable interface.
 *        We intercept this to allow fire stations to handle clicks on actors within the boundary
 * \param ImpactPoint The point where the select event occurred
 * \param bPrimary True if primary select, False if Secondary
 * \param HitActor The actor within the selection that was hit by the select event
 */
void AWfFireStationBase::EventMouseSelect(const FVector& ImpactPoint, bool bPrimary, AActor* HitActor)
{
	if (IsValid(HitActor))
	{
	}
}

void AWfFireStationBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOGFMT(LogTemp, Log, "Actor {OverlapActor} has Entered Fire Station '{MyStation}'"
    	, OtherActor->GetName(), this->GetName());
    if (OtherActor->GetClass()->ImplementsInterface(UWfFireStationInterface::StaticClass()))
    {
    	IWfFireStationInterface* FireStationInterface = Cast<IWfFireStationInterface>(OtherActor);
    	if (FireStationInterface->IsEligible())
    	{
    		FireStationInterface->EventBeginOverlap(this);
	    }
    }
}

void AWfFireStationBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    UE_LOGFMT(LogTemp, Log, "Actor {OverlapActor} has left Fire Station '{MyStation}'"
    	, OtherActor->GetName(), this->GetName());
    if (OtherActor->GetClass()->ImplementsInterface(UWfFireStationInterface::StaticClass()))
    {
    	IWfFireStationInterface* FireStationInterface = Cast<IWfFireStationInterface>(OtherActor);
    	if (FireStationInterface->IsEligible())
    	{
    		FireStationInterface->EventEndOverlap(this);
    	}
    }
}
