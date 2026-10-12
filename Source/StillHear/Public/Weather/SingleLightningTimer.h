// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingleLightningBase.h"
#include "SingleLightningTimer.generated.h"

UCLASS()
class STILLHEAR_API ASingleLightningTimer : public ASingleLightningBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer")
	bool bIsTimeRandom = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (EditCondition = "!bIsTimeRandom", EditConditionHides, ClampMin = 0.1f, UIMin = 0.1f))
	float Time;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (EditCondition = "bIsTimeRandom", EditConditionHides, ClampMin = 0.1f, UIMin = 0.1f))
	float MinTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (EditCondition = "bIsTimeRandom", EditConditionHides, ClampMin = 0.1f, UIMin = 0.1f))
	float MaxTime;
	
	UPROPERTY()
	FTimerHandle LightningTimerHandle;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	ASingleLightningTimer();
#pragma endregion 

#pragma region METHODS
protected:
	virtual void BeginPlay() override;

	virtual void TriggerLightning() override;
	virtual void SetNextLightning();
#pragma endregion 
};
