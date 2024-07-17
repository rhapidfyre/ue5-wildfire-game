// Fill out your copyright notice in the Description page of Project Settings.


#include "Statics/WfUtilities.h"

#include "Logging/StructuredLog.h"

UWfUtilities::UWfUtilities()
{

}

FString UWfUtilities::GenerateGUID()
{
	const FGuid Guid = FGuid::NewGuid();
	const FString NewGuid = Guid.ToString();
	UE_LOGFMT(LogTemp, Display, "Generated GUID '{NewGuid}'", NewGuid);
	return NewGuid;
}
