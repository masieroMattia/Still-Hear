// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/RainyWeatherVolume.h"
#include "NiagaraComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/PostProcessComponent.h"
#include "Data/DataAssets/RainyWeatherDataAsset.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Weather/RainyWeatherParameterNames.h"

ARainyWeatherVolume::ARainyWeatherVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Volume = CreateDefaultSubobject<UBoxComponent>("RainyWeatherVolume");
	RootComponent = Volume;
	
	Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Volume->SetCollisionResponseToAllChannels(ECR_Overlap);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>("PostProcess");
	PostProcess->SetupAttachment(Volume);

	RainVFX = CreateDefaultSubobject<UNiagaraComponent>("RainVFX");
	RainVFX->SetupAttachment(Volume);

	LightningArea = CreateDefaultSubobject<USceneComponent>("ThunderArea");
	LightningArea->SetupAttachment(Volume);
	
	LightningPlane = CreateDefaultSubobject<UStaticMeshComponent>("ThunderPlane");
	LightningPlane->SetupAttachment(LightningArea);
}

void ARainyWeatherVolume::Activate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(OtherActor);

	if (!MainCharacter)
	{
		return;
	}

	CharacterInVolume = MainCharacter;
	
	if (!IsValid(RainyWeatherDataAsset))
	{
		return;
	}

	bDeactivate = false;
	bActivate = true;

	if (RainyWeatherDataAsset->bFollowPlayer)
	{
		bRainFollowPlayer = true;
		bLightningFollowPlayer = true;
	}
	
	// Rain Section
	if (RainyWeatherDataAsset->bDelayRain)
	{
		GetWorldTimerManager().SetTimer(
			RainTimerHandle,
			this,
			&ThisClass::StartRain,
			RainyWeatherDataAsset->RainDelayTime,
			false
		);
	}
	else
	{
		StartRain();
	}

	// Wind Section
	if (RainyWeatherDataAsset->bActivateWind)
	{
		if (RainyWeatherDataAsset->bDelayWind)
		{
			GetWorldTimerManager().SetTimer(
				WindTimerHandle,
				this,
				&ThisClass::StartWind,
				RainyWeatherDataAsset->WindDelayTime,
				false
			);
		}
		else
		{
			StartWind();
		}
	}

	// Lightnings Section
	if (RainyWeatherDataAsset->bActivateLightnings)
	{
		if (RainyWeatherDataAsset->bDelayLightnings)
		{
			GetWorldTimerManager().SetTimer(
				LightningStartTimerHandle,
				this,
				&ThisClass::TriggerLightning,
				RainyWeatherDataAsset->LightningsDelayTime,
				false
			);
		}
		else
		{
			TriggerLightning();
		}
	}

	// Volumetric Clouds Section
	if (RainyWeatherDataAsset->bModifyVolumetricClouds)
	{
		if (RainyWeatherDataAsset->bDelayVolumetricCloudsChange)
		{
			GetWorldTimerManager().SetTimer(
				VolumetricCloudsTimerHandle,
				this,
				&ThisClass::ChangeVolumetricClouds,
				RainyWeatherDataAsset->VolumetricCloudsDelayTime,
				false
			);
		}
		else
		{
			ChangeVolumetricClouds();
		}
	}
	
}

void ARainyWeatherVolume::Deactivate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bCinematicMode)
	{
		return;
	}

	AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(OtherActor);

	if (!MainCharacter)
	{
		return;
	}

	CharacterInVolume = nullptr;

	bActivate = false;
	bDeactivate = true;

	bRainFollowPlayer = false;
	
	// Rain Section
	if (RainyWeatherDataAsset->bDelayExitRain)
	{
		RainTimerHandle.Invalidate();
		
		GetWorldTimerManager().SetTimer(
			RainTimerHandle,
			this,
			&ThisClass::StopRain,
			RainyWeatherDataAsset->ExitRainDelayTime,
			false
		);
	}
	else
	{
		StopRain();
	}

	// Wind Section
	if (RainyWeatherDataAsset->bActivateWind)
	{
		if (RainyWeatherDataAsset->bDelayExitWind)
		{
			WindTimerHandle.Invalidate();
			
			GetWorldTimerManager().SetTimer(
				WindTimerHandle,
				this,
				&ThisClass::StopWind,
				RainyWeatherDataAsset->ExitWindDelayTime,
				false
			);
		}
		else
		{
			StopWind();
		}
	}

	// Lightnings Section
	if (RainyWeatherDataAsset->bActivateLightnings)
	{
		if (RainyWeatherDataAsset->bDelayExitLightnings)
		{
			LightningStartTimerHandle.Invalidate();
			
			GetWorldTimerManager().SetTimer(
				LightningStartTimerHandle,
				this,
				&ThisClass::StopLightnings,
				RainyWeatherDataAsset->ExitLightningsDelayTime,
				false
			);
		}
		else
		{
			StopLightnings();
		}
	}

	// Volumetric Clouds Section
	if (RainyWeatherDataAsset->bModifyVolumetricClouds)
	{
		if (RainyWeatherDataAsset->bDelayExitVolumetricCloudsChange)
		{
			VolumetricCloudsTimerHandle.Invalidate();
			
			GetWorldTimerManager().SetTimer(
				VolumetricCloudsTimerHandle,
				this,
				&ThisClass::ResetVolumetricClouds,
				RainyWeatherDataAsset->ExitVolumetricCloudsDelayTime,
				false
			);
		}
		else
		{
			ResetVolumetricClouds();
		}
	}
}

void ARainyWeatherVolume::MoveLightnings() const
{
	if (IsValid(CharacterInVolume))
	{
		LightningArea->SetWorldLocation(CharacterInVolume->GetActorLocation() + RainyWeatherDataAsset->LightningsOffset);
	}
}

void ARainyWeatherVolume::MoveRain() const
{
	if (IsValid(CharacterInVolume))
	{
		RainVFX->SetWorldLocation(CharacterInVolume->GetActorLocation() + RainyWeatherDataAsset->RainOffset);
	}
}

void ARainyWeatherVolume::SetNextLightningTime()
{
	if (bDeactivate)
	{
		return;
	}

	const float Delay = FMath::RandRange(RainyWeatherDataAsset->MinTimeBetweenLightnings, RainyWeatherDataAsset->MaxTimeBetweenLightnings);

	GetWorldTimerManager().SetTimer(
		LightningCycleTimerHandle,
		this,
		&ThisClass::TriggerLightning,
		Delay,
		false
	);
}

#if WITH_EDITOR
void ARainyWeatherVolume::DrawDebugThunderArea()
{
	if (IsValid(RainyWeatherDataAsset) && RainyWeatherDataAsset->bShowAreaExtents)
	{
		DrawDebugBox(
			GetWorld(),
			LightningArea->GetComponentLocation(),
			FVector(RainyWeatherDataAsset->AreaExtents.X, RainyWeatherDataAsset->AreaExtents.Y, 1),
			LightningArea->GetComponentQuat(),
			RainyWeatherDataAsset->LinesColor,
			false,
			0.1f,
			0,
			RainyWeatherDataAsset->LinesThickness
		);
	}
}

void ARainyWeatherVolume::DrawDebugRainArea()
{
	if (IsValid(RainyWeatherDataAsset) && RainyWeatherDataAsset->bShowAreaExtents)
	{
		DrawDebugBox(
			GetWorld(),
			RainVFX->GetComponentLocation(),
			FVector(RainyWeatherDataAsset->RainBoxSize.X/2, RainyWeatherDataAsset->RainBoxSize.Y/2, 1),
			RainVFX->GetComponentQuat(),
			RainyWeatherDataAsset->LinesColor,
			false,
			0.1f,
			0,
			RainyWeatherDataAsset->LinesThickness
		);
	}
}
#endif

void ARainyWeatherVolume::ChangeVolumetricClouds()
{
	if (RainyWeatherDataAsset->bBlendVolumetricClouds)
	{
		bLerpingClouds = true;
		return;
	}

	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::StormClouds, RainyWeatherDataAsset->StormClouds);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, RainyWeatherDataAsset->Storm_LightningTexScale);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, RainyWeatherDataAsset->LightningFlicker);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, RainyWeatherDataAsset->DynamicLightingAnim);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, RainyWeatherDataAsset->ManualLightningAnim);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SourcePower, RainyWeatherDataAsset->SourcePower);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatter, RainyWeatherDataAsset->FillScatter);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, RainyWeatherDataAsset->FillScatterIntensity);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, RainyWeatherDataAsset->SecondMipLevel);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, RainyWeatherDataAsset->LightningMaskBias);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, RainyWeatherDataAsset->LightningMaskStrength);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, RainyWeatherDataAsset->CloudTextureWeight);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, RainyWeatherDataAsset->Storm_LightningColor);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, RainyWeatherDataAsset->Storm_AlbedoColor);
}

void ARainyWeatherVolume::LerpClouds(const float DeltaTime)
{
	if (bActivate)
	{
		VolumetricCloudTime += DeltaTime;
	}
	else if (bDeactivate)
	{
		VolumetricCloudTime -= DeltaTime;
	}
	
	const float Alpha = VolumetricCloudTime / RainyWeatherDataAsset->CloudsTimeToBlend;

	const float CurrentStormClouds = FMath::Lerp(StormClouds, RainyWeatherDataAsset->StormClouds, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::StormClouds, CurrentStormClouds);

	const float CurrentStorm_LightningTexScale = FMath::Lerp(Storm_LightningTexScale, RainyWeatherDataAsset->Storm_LightningTexScale, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, CurrentStorm_LightningTexScale);

	const float CurrentLightningFlicker = FMath::Lerp(LightningFlicker, RainyWeatherDataAsset->LightningFlicker, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, CurrentLightningFlicker);

	const float CurrentDynamicLightingAnim = FMath::Lerp(DynamicLightingAnim, RainyWeatherDataAsset->DynamicLightingAnim, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, CurrentDynamicLightingAnim);

	const float CurrentManualLightningAnim = FMath::Lerp(ManualLightningAnim, RainyWeatherDataAsset->ManualLightningAnim, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, CurrentManualLightningAnim);

	const float CurrentSourcePower = FMath::Lerp(SourcePower, RainyWeatherDataAsset->SourcePower, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SourcePower, CurrentSourcePower);

	const float CurrentFillScatter = FMath::Lerp(FillScatter, RainyWeatherDataAsset->FillScatter, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatter, CurrentFillScatter);

	const float CurrentFillScatterIntensity = FMath::Lerp(FillScatterIntensity, RainyWeatherDataAsset->FillScatterIntensity, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, CurrentFillScatterIntensity);

	const float CurrentSecondMipLevel = FMath::Lerp(SecondMipLevel, RainyWeatherDataAsset->SecondMipLevel, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, CurrentSecondMipLevel);

	const float CurrentLightningMaskBias = FMath::Lerp(LightningMaskBias, RainyWeatherDataAsset->LightningMaskBias, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, CurrentLightningMaskBias);

	const float CurrentLightningMaskStrength = FMath::Lerp(LightningMaskStrength, RainyWeatherDataAsset->LightningMaskStrength, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, CurrentLightningMaskStrength);

	const float CurrentCloudTextureWeight = FMath::Lerp(CloudTextureWeight, RainyWeatherDataAsset->CloudTextureWeight, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, CurrentCloudTextureWeight);

	const FLinearColor CurrentStorm_LightningColor = FMath::Lerp(Storm_LightningColor, RainyWeatherDataAsset->Storm_LightningColor, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, CurrentStorm_LightningColor);

	const FLinearColor CurrentStorm_AlbedoColor = FMath::Lerp(Storm_AlbedoColor, RainyWeatherDataAsset->Storm_AlbedoColor, Alpha);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, CurrentStorm_AlbedoColor);

	if (Alpha >= 1.0f)
	{
		VolumetricCloudTime = RainyWeatherDataAsset->CloudsTimeToBlend;
		bLerpingClouds = false;
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::StormClouds, RainyWeatherDataAsset->StormClouds);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, RainyWeatherDataAsset->Storm_LightningTexScale);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, RainyWeatherDataAsset->LightningFlicker);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, RainyWeatherDataAsset->DynamicLightingAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, RainyWeatherDataAsset->ManualLightningAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SourcePower, RainyWeatherDataAsset->SourcePower);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatter, RainyWeatherDataAsset->FillScatter);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, RainyWeatherDataAsset->FillScatterIntensity);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, RainyWeatherDataAsset->SecondMipLevel);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, RainyWeatherDataAsset->LightningMaskBias);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, RainyWeatherDataAsset->LightningMaskStrength);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, RainyWeatherDataAsset->CloudTextureWeight);
		VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, RainyWeatherDataAsset->Storm_LightningColor);
		VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, RainyWeatherDataAsset->Storm_AlbedoColor);
	}
	else if (Alpha <= 0.0f)
	{
		VolumetricCloudTime = RainyWeatherDataAsset->CloudsTimeToBlend;
		bLerpingClouds = false;
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::StormClouds, StormClouds);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, Storm_LightningTexScale);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, LightningFlicker);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, DynamicLightingAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, ManualLightningAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SourcePower, SourcePower);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatter, FillScatter);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, FillScatterIntensity);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, SecondMipLevel);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, LightningMaskBias);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, LightningMaskStrength);
		VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, CloudTextureWeight);
		VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, Storm_LightningColor);
		VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, Storm_AlbedoColor);
	}
}

void ARainyWeatherVolume::ResetVolumetricClouds()
{
	if (RainyWeatherDataAsset->bBlendExitVolumetricClouds)
	{
		bLerpingClouds = true;
		return;
	}

	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::StormClouds, StormClouds);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, Storm_LightningTexScale);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, LightningFlicker);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, DynamicLightingAnim);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, ManualLightningAnim);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SourcePower, SourcePower);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatter, FillScatter);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, FillScatterIntensity);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, SecondMipLevel);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, LightningMaskBias);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, LightningMaskStrength);
	VolumetricCloudsMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, CloudTextureWeight);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, Storm_LightningColor);
	VolumetricCloudsMaterialParameterCollectionInstance->SetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, Storm_AlbedoColor);
}

void ARainyWeatherVolume::TriggerLightning()
{
	if (bDeactivate)
	{
		return;
	}
	
	const float TextureIndex = FMath::RandRange(0, LightningTexturesAmount - 1);
	LightningMaterialInstance->SetTextureParameterValue(RainyWeatherParameterNames::LightningTexture, LightningTextures[TextureIndex]);
	
	LightningPlane->SetRelativeLocation(FVector(
		FMath::RandRange(-RainyWeatherDataAsset->AreaExtents.X/2, RainyWeatherDataAsset->AreaExtents.X/2),
		FMath::RandRange(-RainyWeatherDataAsset->AreaExtents.Y/2, RainyWeatherDataAsset->AreaExtents.Y/2),
		0
	));

	const FVector Direction = (CharacterInVolume->GetActorLocation() - LightningPlane->GetComponentLocation()).GetSafeNormal();
	const FRotator LookAtRotation = FRotator(0, Direction.Rotation().Yaw, 90);
	LightningPlane->SetRelativeRotation(LookAtRotation);

	if (RainyWeatherDataAsset->bScreenFlash)
	{
		PostProcess->Settings.AutoExposureBias = RainyWeatherDataAsset->PostProcessExposureDuringLightning;
	}

	bLightningFollowPlayer = false;
	bTriggerLightning = true;
}

void ARainyWeatherVolume::GoThroughLightningCurve(const float DeltaTime)
{
	CurrentLightningTime += DeltaTime;
	const float Alpha = RainyWeatherDataAsset->LightningAppearanceCurve->GetFloatValue(CurrentLightningTime);
	
	LightningMaterialInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningPercentage, Alpha);

	if (RainyWeatherDataAsset->bScreenFlash && RainyWeatherDataAsset->FlashDuration != 0.0f && CurrentLightningTime >= RainyWeatherDataAsset->FlashDuration)
	{
		PostProcess->Settings.AutoExposureBias = InitialPostProcessExposure;
	}

	if (CurrentLightningTime >= CurveLightningTime)
	{
		CurrentLightningTime = 0;
		LightningMaterialInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningPercentage, RainyWeatherDataAsset->LightningMinimumValueToDisappear);
		PostProcess->Settings.AutoExposureBias = InitialPostProcessExposure;
		bTriggerLightning = false;
		bLightningFollowPlayer = true;
		SetNextLightningTime();
	}
}

void ARainyWeatherVolume::StopLightnings()
{
	LightningCycleTimerHandle.Invalidate();
}

void ARainyWeatherVolume::StartRain()
{
	if (RainyWeatherDataAsset->bSmoothRainIntensity)
	{
		bLerpingRain = true;
		return;
	}

	RainVFX->SetVariableFloat(RainyWeatherParameterNames::SpawnRate, TrueMaxSpawnRate);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Rain, RainyWeatherDataAsset->RainMaxIntensity);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wetness, RainyWeatherDataAsset->RainMaxIntensity);
}

void ARainyWeatherVolume::LerpRain(const float DeltaTime)
{
	if (bActivate)
	{
		LerpingRainTime += DeltaTime;
	}
	else if (bDeactivate)
	{
		LerpingRainTime -= DeltaTime;
	}
	float Alpha = LerpingRainTime / RainyWeatherDataAsset->RainTimeToReachMaxIntensity;
	Alpha = FMath::Pow(Alpha, 4);
	const float CurrentSpawnRate = FMath::Lerp(0, TrueMaxSpawnRate, Alpha);
	RainVFX->SetVariableFloat(RainyWeatherParameterNames::SpawnRate, CurrentSpawnRate);
	const float CurrentIntensity = FMath::Lerp(0, RainyWeatherDataAsset->RainMaxIntensity, Alpha);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Rain, CurrentIntensity);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wetness, CurrentIntensity);

	if (Alpha >= 1.0f)
	{
		LerpingRainTime = RainyWeatherDataAsset->RainTimeToReachMaxIntensity;
		RainVFX->SetVariableFloat(RainyWeatherParameterNames::SpawnRate, TrueMaxSpawnRate);
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Rain, RainyWeatherDataAsset->RainMaxIntensity);
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wetness, RainyWeatherDataAsset->RainMaxIntensity);
		bLerpingRain = false;
	}
	else if (bDeactivate && Alpha <= 0.05f)
	{
		LerpingRainTime = 0;
		RainVFX->SetVariableFloat(RainyWeatherParameterNames::SpawnRate, 0);
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Rain, 0);
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wetness, 0);
		bLerpingRain = false;
	}
}

void ARainyWeatherVolume::StopRain()
{
	if (RainyWeatherDataAsset->bSmoothRainExit)
	{
		bLerpingRain = true;
		return;
	}

	RainVFX->SetVariableFloat(RainyWeatherParameterNames::SpawnRate, 0);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Rain, 0);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wetness, 0);
}

void ARainyWeatherVolume::StartWind()
{
	if (RainyWeatherDataAsset->bSmoothWindIntensity)
	{
		bLerpingWind = true;
		return;
	}

	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wind, RainyWeatherDataAsset->WindMaxIntensity);
}

void ARainyWeatherVolume::LerpWind(const float DeltaTime)
{
	if (bActivate)
	{
		LerpingWindTime += DeltaTime;
	}
	else if (bDeactivate)
	{
		LerpingWindTime -= DeltaTime;
	}
	const float Alpha = LerpingWindTime / RainyWeatherDataAsset->WindTimeToReachMaxIntensity;
	// Not sure if I should include this calculation for the wind too
	// Alpha = FMath::Pow(Alpha, 4);
	const float CurrentIntensity = FMath::Lerp(0, RainyWeatherDataAsset->WindMaxIntensity, Alpha);
	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wind, CurrentIntensity);

	if (Alpha >= 1.0f)
	{
		LerpingWindTime = RainyWeatherDataAsset->WindTimeToReachMaxIntensity;
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wind, RainyWeatherDataAsset->WindMaxIntensity);
		bLerpingWind = false;
	}
	else if (Alpha <= 0.0f)
	{
		LerpingWindTime = 0;
		RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wind, 0);
		bLerpingWind = false;
	}
}

void ARainyWeatherVolume::StopWind()
{
	if (RainyWeatherDataAsset->bSmoothWindExit)
	{
		bLerpingWind = true;
		return;
	}

	RainMaterialParameterCollectionInstance->SetScalarParameterValue(RainyWeatherParameterNames::Wind, 0);
}

void ARainyWeatherVolume::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (!IsValid(RainyWeatherDataAsset))
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("RainyWeatherDataAsset is not assigned in ") + GetName());
		return;
	}

	if (!IsValid(RainMaterialParameterCollection))
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("RainMaterialParameterCollection is not assigned in ") + GetName());
		return;
	}
#endif

	Volume->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::Activate);
	Volume->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::Deactivate);
	
	// Rain
	TrueMaxSpawnRate = RainyWeatherDataAsset->RainMaxSpawnRate * RainyWeatherDataAsset->RainMaxIntensity;
	RainVFX->SetVariableVec3(RainyWeatherParameterNames::Velocity, RainyWeatherDataAsset->RainVelocity);
	RainVFX->SetVariableVec2(RainyWeatherParameterNames::SpriteSize, RainyWeatherDataAsset->RainSpriteSize);
	RainMaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(RainMaterialParameterCollection);
	CurveLightningTime = RainyWeatherDataAsset->LightningAppearanceCurve->FloatCurve.GetLastKey().Time;

	// Lightning
	LightningMaterialInstance = LightningPlane->CreateDynamicMaterialInstance(0);
	LightningMaterialInstance->SetScalarParameterValue(RainyWeatherParameterNames::LightningPercentage, RainyWeatherDataAsset->LightningMinimumValueToDisappear);
	LightningMaterialInstance->SetScalarParameterValue(RainyWeatherParameterNames::EmissiveMultiplier, RainyWeatherDataAsset->LightningEmissiveMultiplier);
	LightningMaterialInstance->SetVectorParameterValue(RainyWeatherParameterNames::LightningColor, RainyWeatherDataAsset->LightningColor);
	LightningTexturesAmount = LightningTextures.Num();

	// PostProcess exposure
	PostProcess->Settings.bOverride_AutoExposureBias = true;
	InitialPostProcessExposure = PostProcess->Settings.AutoExposureBias;

	// Clouds
	if (RainyWeatherDataAsset->bModifyVolumetricClouds)
	{
		VolumetricCloudsMaterialParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(VolumetricCloudsMaterialParameterCollection);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::StormClouds, StormClouds);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::Storm_LightningTexScale, Storm_LightningTexScale);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::LightningFlicker, LightningFlicker);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::DynamicLightingAnim, DynamicLightingAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::ManualLightningAnim, ManualLightningAnim);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::SourcePower, SourcePower);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::FillScatter, FillScatter);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::FillScatterIntensity, FillScatterIntensity);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::SecondMipLevel, SecondMipLevel);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::LightningMaskBias, LightningMaskBias);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::LightningMaskStrength, LightningMaskStrength);
		VolumetricCloudsMaterialParameterCollectionInstance->GetScalarParameterValue(RainyWeatherParameterNames::CloudTextureWeight, CloudTextureWeight);
		VolumetricCloudsMaterialParameterCollectionInstance->GetVectorParameterValue(RainyWeatherParameterNames::Storm_LightningColor, Storm_LightningColor);
		VolumetricCloudsMaterialParameterCollectionInstance->GetVectorParameterValue(RainyWeatherParameterNames::Storm_AlbedoColor, Storm_AlbedoColor);
	}
}

void ARainyWeatherVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	RainVFX->SetRelativeLocation(FVector(0, 0, Volume->GetScaledBoxExtent().Z));
	
	if (IsValid(RainyWeatherDataAsset))
	{		
		if (RainyWeatherDataAsset->bPersonalizeRainBounds)
		{
			RainVFX->SetVectorParameter(RainyWeatherParameterNames::BoxSize, RainyWeatherDataAsset->RainBoxSize);
		}
		else
		{
			RainVFX->SetVectorParameter(RainyWeatherParameterNames::BoxSize, FVector(Volume->GetScaledBoxExtent().X * 2, Volume->GetScaledBoxExtent().Y * 2, 0));
		}
	}
#endif
	
}

#if WITH_EDITOR
bool ARainyWeatherVolume::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void ARainyWeatherVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRainFollowPlayer)
	{
		MoveRain();
	}

	if (bLightningFollowPlayer)
	{
		MoveLightnings();
	}
	
	if (bLerpingRain)
	{
		LerpRain(DeltaTime);
	}

	if (bLerpingWind)
	{
		LerpWind(DeltaTime);
	}

	if (bTriggerLightning)
	{
		GoThroughLightningCurve(DeltaTime);
	}

	if (bLerpingClouds)
	{
		LerpClouds(DeltaTime);
	}

#if WITH_EDITOR
	DrawDebugThunderArea();
	DrawDebugRainArea();
#endif
	
}

