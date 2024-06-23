// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"


#include "WfHudBase.generated.h"

class UUserWidget;

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTWILDFIRE_API AWfHudBase : public AHUD
{
	GENERATED_BODY()

public:

	AWfHudBase();

	UFUNCTION(BlueprintCallable)
	void SetWidgetHoldingControl(UUserWidget* WidgetReference, bool bHasControl);

	UFUNCTION(BlueprintCallable)
	TArray<UUserWidget*> GetWidgetsHoldingControl() const;

	UFUNCTION(BlueprintPure)
	bool IsAnyWidgetHoldingControl() const { return WidgetsHoldingControl.Num() > 0; }

private:

	TSet<UUserWidget*> WidgetsHoldingControl;
};
