// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "WfFireStationBase.generated.h"

class AWfFfCharacterBase;
class AWfVehicleBase;

/**
 * \brief Defines the base class of a fire station
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTWILDFIRE_API AWfFireStationBase : public AActor
{
	GENERATED_BODY()

public:

	AWfFireStationBase();

	void SetFireStationOwner(const APlayerController* PlayerController);
	const FString& GetFireStationOwner() const { return FireStationOwner; }

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

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_FireStationOwner(const FString& OldOwner);


	UPROPERTY(ReplicatedUsing=OnRep_FireStationOwner)
	FString FireStationOwner;
};
