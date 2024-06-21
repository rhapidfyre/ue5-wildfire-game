// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WfAiControllerBase.h"


// Sets default values
AWfAiControllerBase::AWfAiControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWfAiControllerBase::BeginPlay()
{
	Super::BeginPlay();

}

void AWfAiControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
