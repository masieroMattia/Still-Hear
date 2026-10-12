// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/SingleLightningTimer.h"


ASingleLightningTimer::ASingleLightningTimer()
{
}

void ASingleLightningTimer::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bIsTimeRandom)
	{
		GetWorldTimerManager().SetTimer(
			LightningTimerHandle,
			this,
			&ThisClass::TriggerLightning,
			Time,
			true
		);
	}
	else
	{
		SetNextLightning();
	}
}

void ASingleLightningTimer::TriggerLightning()
{
	Super::TriggerLightning();

	if (bIsTimeRandom)
	{
		SetNextLightning();
	}
}

void ASingleLightningTimer::SetNextLightning()
{
	if (!GetWorld()) 
		return;

	const float RandomTime = FMath::RandRange(MinTime, MaxTime);

	GetWorldTimerManager().SetTimer(
		LightningTimerHandle,
		this,
		&ThisClass::TriggerLightning,
		RandomTime,
		false
	);
}


