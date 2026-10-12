#include "Camera/CameraEffects/CameraEffectsComponent.h"

#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Shakes/LegacyCameraShake.h"
#include "Camera/CameraEffects/CameraFOVModifier.h"
#include "Camera/CameraEffects/CameraOffsetModifier.h"

#pragma region CONSTRUCTOR
UCameraEffectsComponent::UCameraEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
#pragma endregion

#pragma region METHODS
void UCameraEffectsComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bAnyFOV = bFOVPulseActive || (SustainedFOVAlpha > 0.0f) || bSustainedFOVActive;
	const bool bAnyOffset = bOffsetPulseActive || bSustainedOffsetActive || !CurrentSustainedOffset.IsNearlyZero(0.01f);

	if (bAnyFOV)
		TickFOV(DeltaTime);
	else if (FOVModifier.IsValid()) 
		FOVModifier->FOVDeltaToApply = 0.0f;
	
	if (bAnyOffset)
		TickOffset(DeltaTime);
	else if (OffsetModifier.IsValid())
		OffsetModifier->OffsetToApply = FVector::ZeroVector;

	if (!bAnyFOV && !bAnyOffset)
		SetComponentTickEnabled(false);
}

void UCameraEffectsComponent::PlayEffectPreset(const FCameraEffectPreset& Preset, FVector CustomEpicenter)
{
	if (Preset.bPlayShake)
		PlayCameraShake(Preset.ShakeConfig, CustomEpicenter);

	if (Preset.bPlayFOV)
		PlayFOV(Preset.FOVConfig);

	if (Preset.bPlayOffset)
		PlayOffset(Preset.OffsetConfig);
}

void UCameraEffectsComponent::PlayCameraShake(const FCameraShakeConfig& Config, FVector CustomEpicenter)
{
	if (!Config.ShakeClass)
		return;

	const APlayerController* PlayerController = GetOwnerController();
	if (!PlayerController)
		return;

	APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager)
		return;
	
	StopLoopingShake();

	FCameraShakeConfig ActiveConfig = Config;
	if (!CustomEpicenter.IsZero())
	{
		ActiveConfig.Epicenter = CustomEpicenter;
		ActiveConfig.bIsLocalShake = false;
	}
	
	if (ActiveConfig.bIsLocalShake)
		ActiveLoopingShake = CameraManager->StartCameraShake(ActiveConfig.ShakeClass, ActiveConfig.Scale);
	else
	{
		UGameplayStatics::PlayWorldCameraShake(
			this,
			ActiveConfig.ShakeClass,
			ActiveConfig.Epicenter,
			ActiveConfig.InnerRadius,
			ActiveConfig.OuterRadius,
			ActiveConfig.Falloff,
			false
		);
	}

	// Play associated force feedback
	if (ActiveConfig.ShakeForceFeedback)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = ActiveConfig.bLoopForceFeedback;
		Params.bIgnoreTimeDilation = true;
		const_cast<APlayerController*>(PlayerController)->ClientPlayForceFeedback(ActiveConfig.ShakeForceFeedback, Params);
		ActiveShakeForceFeedback = ActiveConfig.ShakeForceFeedback;

		// Automatically stop force feedback after the camera shake's duration expires
		if (GetWorld() && ActiveConfig.ShakeClass)
		{
			if (const UCameraShakeBase* ShakeCDO = ActiveConfig.ShakeClass->GetDefaultObject<UCameraShakeBase>())
			{
				float Duration = 0.0f;
				if (const ULegacyCameraShake* LegacyCDO = Cast<ULegacyCameraShake>(ShakeCDO))
				{
					if (LegacyCDO->OscillationDuration > 0.0f)
					{
						Duration = LegacyCDO->OscillationDuration + LegacyCDO->OscillationBlendOutTime;
					}
				}
				else
				{
					const FCameraShakeDuration DurationObj = ShakeCDO->GetCameraShakeDuration();
					if (DurationObj.IsFixed())
					{
						Duration = DurationObj.Get();
					}
				}

				if (Duration > 0.0f)
				{
					GetWorld()->GetTimerManager().SetTimer(
						ForceFeedbackTimerHandle,
						this,
						&UCameraEffectsComponent::StopLoopingShake,
						Duration,
						false
					);
				}
			}
		}
	}
}

// Captures the camera's current FOV, then drives it through a curve
void UCameraEffectsComponent::PlayFOV(const FCameraFOVConfig& Config)
{
	APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager)
		return;
	
	if (!FOVModifier.IsValid()) 
		FOVModifier = Cast<UCameraFOVModifier>(CameraManager->AddNewCameraModifier(UCameraFOVModifier::StaticClass()));
	
	if (Config.bKeepFOV)
	{
		if (ActiveSustainedFOVConfig.PulseCurve != Config.PulseCurve)
		{
			if (Config.PulseCurve)
			{
				float MinT, MaxT;
				Config.PulseCurve->GetTimeRange(MinT, MaxT);
				SustainedFOVAlpha = FMath::Clamp(SustainedFOVAlpha, MinT, MaxT);
			}
		}
		
		SustainedFOVDelta = Config.FOVDelta;
		ActiveSustainedFOVConfig = Config;
		bSustainedFOVActive = true;
	}
	else if (Config.PulseCurve)
	{
		ActiveFOVPulses.Add({ Config, 0.0f });
		bFOVPulseActive = true;
	}

	SetComponentTickEnabled(true);
}

// Applies an instantaneous positional kick to the camera, then springs it back to origin over time
void UCameraEffectsComponent::PlayOffset(const FCameraOffsetConfig& Config)
{
	APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager)
		return;

	if (!OffsetModifier.IsValid())
		OffsetModifier = Cast<UCameraOffsetModifier>(CameraManager->AddNewCameraModifier(UCameraOffsetModifier::StaticClass()));
	
	const FVector WorldKick = CameraManager->GetCameraRotation().RotateVector(Config.KickDirection);

	if (Config.bKeepOffset)
	{
		SustainedOffset += WorldKick;
		ActiveSustainedOffsetConfig = Config;
		bSustainedOffsetActive = true;
	}
	else
	{
		ActiveOffsetPulses.Add({ Config, WorldKick, 0.0f });
		bOffsetPulseActive = true;
	}

	SetComponentTickEnabled(true);
}

APlayerCameraManager* UCameraEffectsComponent::GetCameraManager() const
{
	const APlayerController* PC = GetOwnerController();
	return 
	PC ? PC->PlayerCameraManager : nullptr;
}

APlayerController* UCameraEffectsComponent::GetOwnerController() const
{
	return Cast<APlayerController>(GetOwner());
}

// Advances the FOV pulse by DeltaTime and writes the new FOV each frame
void UCameraEffectsComponent::TickFOV(const float DeltaTime)
{
	if (!FOVModifier.IsValid()) 
		return;

	float TotalDelta = 0.0f;
	
	if (ActiveSustainedFOVConfig.PulseCurve)
	{
		float MinT, MaxT;
		ActiveSustainedFOVConfig.PulseCurve->GetTimeRange(MinT, MaxT);
        
		// Advance or retreat the alpha time based on active status
		const float TargetTime = bSustainedFOVActive ? MaxT : 0.0f;
		SustainedFOVAlpha = FMath::FInterpConstantTo(SustainedFOVAlpha, TargetTime, DeltaTime, 1.0f);

		const float Weight = ActiveSustainedFOVConfig.PulseCurve->GetFloatValue(SustainedFOVAlpha);
		TotalDelta += SustainedFOVDelta * Weight;
		
		if (!bSustainedFOVActive && FMath::IsNearlyZero(SustainedFOVAlpha))
			SustainedFOVDelta = 0.0f; // Reset the accumulation once the visual return is done
	}
	else if (bSustainedFOVActive || SustainedFOVAlpha > 0.0f)
	{
		// Fallback if no curve is provided (linear interp)
		SustainedFOVAlpha = FMath::FInterpTo(SustainedFOVAlpha, bSustainedFOVActive ? 1.0f : 0.0f, DeltaTime, 5.0f);
		TotalDelta += SustainedFOVDelta * SustainedFOVAlpha;
		
		if (!bSustainedFOVActive && FMath::IsNearlyZero(SustainedFOVAlpha))
			SustainedFOVDelta = 0.0f;
	}
	
	for (int32 i = ActiveFOVPulses.Num() - 1; i >= 0; --i)
	{
		FActiveFOVPulse& Pulse = ActiveFOVPulses[i];
		Pulse.Alpha += DeltaTime;

		if (Pulse.Config.PulseCurve)
		{
			float MinT, MaxT;
			Pulse.Config.PulseCurve->GetTimeRange(MinT, MaxT);
			
			TotalDelta += Pulse.Config.FOVDelta * Pulse.Config.PulseCurve->GetFloatValue(Pulse.Alpha);

			if (Pulse.Alpha >= MaxT)
				ActiveFOVPulses.RemoveAt(i);
		}
		else
		{
			ActiveFOVPulses.RemoveAt(i);
		}
	}

	bFOVPulseActive = ActiveFOVPulses.Num() > 0;

	FOVModifier->FOVDeltaToApply = TotalDelta;
}

// Springs CurrentOffset toward zero each frame and applies it to the camera
void UCameraEffectsComponent::TickOffset(const float DeltaTime)
{
	const FVector TargetSustained = bSustainedOffsetActive ? SustainedOffset : FVector::ZeroVector;
	
	CurrentSustainedOffset = FMath::VInterpTo(
		CurrentSustainedOffset,
		TargetSustained,
		DeltaTime,
		ActiveSustainedOffsetConfig.SpringSpeed
	);
	
	FVector TotalOffset = CurrentSustainedOffset;

	for (int32 i = ActiveOffsetPulses.Num() - 1; i >= 0; --i)
	{
		FActiveOffsetPulse& Pulse = ActiveOffsetPulses[i];
		Pulse.ElapsedTime += DeltaTime;
		Pulse.CurrentOffset = FMath::VInterpTo(Pulse.CurrentOffset, FVector::ZeroVector, DeltaTime, Pulse.Config.SpringSpeed);

		const bool bExpired = Pulse.ElapsedTime >= Pulse.Config.MaxDuration;
		const bool bFinished = Pulse.CurrentOffset.IsNearlyZero(0.01f);

		TotalOffset += Pulse.CurrentOffset;

		if (bExpired || bFinished)
			ActiveOffsetPulses.RemoveAt(i);
	}

	bOffsetPulseActive = ActiveOffsetPulses.Num() > 0;

	if (OffsetModifier.IsValid())
		OffsetModifier->OffsetToApply = TotalOffset;
}

void UCameraEffectsComponent::StopLoopingShake()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ForceFeedbackTimerHandle);
	}

	if (ActiveLoopingShake)
	{
		if (APlayerCameraManager* CameraManager = GetCameraManager())
			CameraManager->StopCameraShake(ActiveLoopingShake, false);
		ActiveLoopingShake = nullptr;
	}

	if (ActiveShakeForceFeedback)
	{
		if (APlayerController* PC = GetOwnerController())
		{
			PC->ClientStopForceFeedback(ActiveShakeForceFeedback, NAME_None);
		}
		ActiveShakeForceFeedback = nullptr;
	}
}

// Immediately terminates every running effect and restores the camera
void UCameraEffectsComponent::StopAllEffects()
{
	StopLoopingShake();
	
	bSustainedFOVActive = false;
	bSustainedOffsetActive = false;
	
	// Clear all active one-shot pulses
	ActiveFOVPulses.Empty();
	ActiveOffsetPulses.Empty();
	
	bFOVPulseActive = false;
	bOffsetPulseActive = false;
}

void UCameraEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ForceFeedbackTimerHandle);
	}

	if (APlayerCameraManager* CameraManager = GetCameraManager())
	{
		if (FOVModifier.IsValid()) 
			CameraManager->RemoveCameraModifier(FOVModifier.Get());
		if (OffsetModifier.IsValid()) 
			CameraManager->RemoveCameraModifier(OffsetModifier.Get());
	}
	Super::EndPlay(EndPlayReason);
}
#pragma endregion
