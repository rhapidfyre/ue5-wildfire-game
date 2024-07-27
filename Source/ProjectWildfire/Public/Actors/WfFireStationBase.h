// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "WfFireStationBase.generated.h"

class AWfPlayerStateBase;
class AWfFfCharacterBase;
class AWfVehicleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireStationNumberChanged, const int, OldNumber, const int, NewNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireStationNameChanged, const FString&, OldName, const FString&, NewName);

UCLASS(BlueprintType, Blueprintable)
class UParkingSpotComponent : public UArrowComponent
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
	int GetFireStationNumber() const { return FireStationNumber; }

	UFUNCTION(BlueprintPure)
	TArray<UParkingSpotComponent*> GetParkingSpots() const { return ParkingSpots; }

	UFUNCTION(BlueprintPure)
	UParkingSpotComponent* GetParkingSpot(const int ParkingSpotNumber) const;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void AddParkingSpot();

	UFUNCTION(BlueprintCallable)
	void DeleteParkingSpot(UParkingSpotComponent* ParkingSpot);

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Settings")
	UBoxComponent* BoundaryBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Settings")
	USceneComponent* SpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Settings")
	TArray<UParkingSpotComponent*> ParkingSpots;

	UPROPERTY(ReplicatedUsing=OnRep_FireStationNumber,
		VisibleAnywhere, BlueprintReadWrite, Category = "Fire Station Settings")
	int FireStationNumber;

	UPROPERTY(BlueprintAssignable) FOnFireStationNumberChanged OnFireStationNumberChanged;
	UPROPERTY(BlueprintAssignable) FOnFireStationNameChanged OnFireStationNameChanged;

private:

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_FireStationNumber(const int OldStationNumber);

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
