// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIInfo_DataAssetBase.h"
#include "AIWormInfo_DataAsset.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UAIWormInfo_DataAsset : public UAIInfo_DataAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vibration Perception Settings")
	float MaxHearingRange = 3000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vibration Perception Settings")
	float RunHearingRange = 750.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vibration Perception Settings")
	float WalkHearingRange = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vibration Perception Settings")
	float CrouchHearingRange = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timers")
	float AlertCooldownTimer = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timers")
	float BellCooldownTimer = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MinDolphinDiveDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MaxDolphinDiveDistance = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool ShowDebugCircles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FColor RunHearingRangeColor = FColor::Yellow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FColor WalkHearingRangeColor = FColor::Orange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FColor CrouchHearingRangeColor = FColor::Red;
};
