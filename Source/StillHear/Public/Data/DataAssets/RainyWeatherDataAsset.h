// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RainyWeatherDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API URainyWeatherDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	bool bFollowPlayer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FVector LightningsOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FVector RainOffset;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain")
	bool bDelayRain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain", meta = (EditCondition = "bDelayRain", EditConditionHides, ClampMin = "0.0"))
	float RainDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float RainMaxIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain")
	bool bSmoothRainIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain", meta = (EditCondition = "bSmoothRainIntensity", EditConditionHides, ClampMin = "0.0"))
	float RainTimeToReachMaxIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain", meta = (ClampMin = "0.0", ClampMax = "10000.0", UIMin = "0.0", UIMax = "10000.0"))
	int RainMaxSpawnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain")
	FVector RainVelocity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain")
	FVector2D RainSpriteSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain")
	bool bPersonalizeRainBounds;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Rain", meta = (EditCondition = "bPersonalizeRainBounds", EditConditionHides))
	FVector RainBoxSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind")
	bool bActivateWind;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind", meta = (EditCondition = "bActivateWind", EditConditionHides))
	bool bDelayWind;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind", meta = (EditCondition = "bActivateWind && bDelayWind", EditConditionHides, ClampMin = "0.0"))
	float WindDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind", meta = (EditCondition = "bActivateWind", EditConditionHides, ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float WindMaxIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind", meta = (EditCondition = "bActivateWind", EditConditionHides))
	bool bSmoothWindIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Wind", meta = (EditCondition = "bActivateWind && bSmoothWindIntensity", EditConditionHides, ClampMin = "0.0"))
	float WindTimeToReachMaxIntensity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings|Settings")
	FVector2D AreaExtents;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings|Settings")
	bool bShowAreaExtents;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings|Settings", meta = (EditCondition = "bShowAreaExtents", EditConditionHides))
	FColor LinesColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings|Settings", meta = (EditCondition = "bShowAreaExtents", EditConditionHides, ClampMin = "0.0"))
	float LinesThickness;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings")
	bool bActivateLightnings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	FLinearColor LightningColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	float LightningEmissiveMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	bool bDelayLightnings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings && bDelayLightnings", EditConditionHides, ClampMin = "0.0"))
	float LightningsDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	float MinTimeBetweenLightnings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	float MaxTimeBetweenLightnings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	TObjectPtr<class UCurveFloat> LightningAppearanceCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	bool bScreenFlash;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bScreenFlash && bActivateLightnings", EditConditionHides))
	float PostProcessExposureDuringLightning;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Lightnings", meta = (EditCondition = "bScreenFlash && bActivateLightnings", EditConditionHides, ToolTip = "Time in which the screen stays white. Starts together with the Lightning and cannot last longer than the lightning. Leave at 0 to have same duration with Lightning"))
	float FlashDuration;
	
	const float LightningMinimumValueToDisappear = -1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds")
	bool bModifyVolumetricClouds;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	bool bDelayVolumetricCloudsChange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bDelayVolumetricCloudsChange && bModifyVolumetricClouds", EditConditionHides, ClampMin = "0.0"))
	float VolumetricCloudsDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	bool bBlendVolumetricClouds;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bBlendVolumetricClouds && bModifyVolumetricClouds", EditConditionHides, ClampMin = "0.0"))
	float CloudsTimeToBlend;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float StormClouds;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float Storm_LightningTexScale;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float LightningFlicker;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float DynamicLightingAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float ManualLightningAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float SourcePower;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float FillScatter;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float FillScatterIntensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float SecondMipLevel;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float LightningMaskBias;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float LightningMaskStrength;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	float CloudTextureWeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	FLinearColor Storm_LightningColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enter|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	FLinearColor Storm_AlbedoColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Rain")
	bool bDelayExitRain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Rain", meta = (EditCondition = "bDelayExitRain", EditConditionHides, ClampMin = "0.0"))
	float ExitRainDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Rain", meta = (EditCondition = "bSmoothRainIntensity", EditConditionHides))
	bool bSmoothRainExit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Wind", meta = (EditCondition = "bActivateWind", EditConditionHides))
	bool bDelayExitWind;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Wind", meta = (EditCondition = "bActivateWind && bDelayExitWind", EditConditionHides, ClampMin = "0.0"))
	float ExitWindDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Wind", meta = (EditCondition = "bActivateWind && bSmoothWindIntensity", EditConditionHides))
	bool bSmoothWindExit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Lightnings", meta = (EditCondition = "bActivateLightnings", EditConditionHides))
	bool bDelayExitLightnings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Lightnings", meta = (EditCondition = "bActivateLightnings && bDelayExitLightnings", EditConditionHides, ClampMin = "0.0"))
	float ExitLightningsDelayTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Clouds", meta = (EditCondition = "bModifyVolumetricClouds", EditConditionHides))
	bool bDelayExitVolumetricCloudsChange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Clouds", meta = (EditCondition = "bDelayExitVolumetricCloudsChange && bModifyVolumetricClouds", EditConditionHides, ClampMin = "0.0"))
	float ExitVolumetricCloudsDelayTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Exit|Clouds", meta = (EditCondition = "bModifyVolumetricClouds && bBlendVolumetricClouds", EditConditionHides))
	bool bBlendExitVolumetricClouds;
};
