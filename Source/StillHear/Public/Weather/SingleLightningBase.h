// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraEffects/CameraEffectTypes.h"
#include "GameFramework/Actor.h"
#include "SingleLightningBase.generated.h"

UCLASS(Abstract)
class STILLHEAR_API ASingleLightningBase : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	TObjectPtr<class USceneComponent> Root;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<class UNiagaraComponent> LightningVfx;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	FCameraEffectPreset CameraEffects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<class USoundBase> LightningSfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SoundDelayTime;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pole")
	TObjectPtr<class AElectrifiedPole> Pole;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	ASingleLightningBase();
#pragma endregion

#pragma region METHODS
protected:
	virtual void TriggerLightning();
#pragma endregion 
};
