// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "WfScheduleComponent.generated.h"


UENUM(BlueprintType)
enum class EScheduleType : uint8
{
	California  UMETA(DisplayName = "California (3-on, 4-off)"),
	Alternating UMETA(DisplayName = "3-on 4-off Alternating"),
	Kelly	    UMETA(DisplayName = "24/48 (Kelly)")

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTWILDFIRE_API UWfScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWfScheduleComponent();

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintPure)
	bool IsOnDuty() const;

	// Which shift this firefighter works (A, B, C, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ScheduledShift = "A";

private:
	TMap< char, TArray<int32> > ShiftSchedule;
};
