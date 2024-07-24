// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "WfFireStationBase.generated.h"

class AWfPlayerStateBase;
class AWfFfCharacterBase;
class AWfVehicleBase;

UCLASS(BlueprintType, Blueprintable)
class UParkingSpotComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UParkingSpotComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Management")
	AWfVehicleBase* AssignedVehicle;

};

/**
 * \brief Defines the base class of a fire station
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfFireStationBase : public AActor
{
	GENERATED_BODY()

public:

	AWfFireStationBase();

	UFUNCTION(BlueprintPure)
	TArray<UParkingSpotComponent*> GetParkingSpots() const { return ParkingSpots; }

	UFUNCTION(BlueprintPure)
	UParkingSpotComponent* GetParkingSpot(const int ParkingSpotNumber) const;

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station")
	UBoxComponent* BoundaryBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station")
	USceneComponent* SpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station")
	TArray<UParkingSpotComponent*> ParkingSpots;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Settings")
	int FireStationNumber;

private:

	UFUNCTION()
	void EventMouseSelect(const FVector& ImpactPoint, bool bPrimary, AActor* HitActor);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
