// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectWildfire/Public/Hud/WfHudBase.h"
#include "Blueprint/UserWidget.h"
#include "Logging/StructuredLog.h"

AWfHudBase::AWfHudBase()
{
}

void AWfHudBase::SetWidgetHoldingControl(UUserWidget* WidgetReference, bool bHasControl)
{

	if (IsValid(WidgetReference))
	{
		if (bHasControl)
			WidgetsHoldingControl.Remove(WidgetReference);
		else
			WidgetsHoldingControl.Add(WidgetReference);

		UE_LOGFMT(LogTemp, Display, "Widget '{WidgetRef}' is {NowStatus} holding control."
			, WidgetReference->GetName(), bHasControl ? "now" : "no longer");
	}
}

TArray<UUserWidget*> AWfHudBase::GetWidgetsHoldingControl() const
{
	TArray<UUserWidget*> HeldWidgets;
	for (auto& WidgetHoldingControl : WidgetsHoldingControl)
	{
		HeldWidgets.Add(WidgetHoldingControl);
	}
	return HeldWidgets;
}
