// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/SingleLightningBase.h"

#include "NiagaraComponent.h"
#include "Camera/CameraEffects/CameraEffectsBlueprintLibrary.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weather/ElectrifiedPole.h"

ASingleLightningBase::ASingleLightningBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	
	LightningVfx = CreateDefaultSubobject<UNiagaraComponent>("LightningVfx");
	LightningVfx->SetupAttachment(RootComponent);
}

void ASingleLightningBase::TriggerLightning()
{
	if (LightningVfx)
	{
		LightningVfx->Activate(true);
	}

	if (Pole)
	{
		Pole->StartEffect();
	}

	if (LightningSfx)
	{
		if (SoundDelayTime > 0.0f)
		{
			FTimerHandle SoundTimerHandle;
			FTimerDelegate SoundDelegate;
			SoundDelegate.BindWeakLambda(this, [this]()
			{
				UGameplayStatics::PlaySoundAtLocation(this, LightningSfx, LightningVfx->GetComponentLocation());
			});
			GetWorldTimerManager().SetTimer(SoundTimerHandle, SoundDelegate, SoundDelayTime, false);
		
			return;
		}

		UGameplayStatics::PlaySoundAtLocation(this, LightningSfx, LightningVfx->GetComponentLocation());	
	}

	UCameraEffectsComponent* CachedCameraEffectComponent = UCameraEffectsBlueprintLibrary::GetCameraEffects(this);

	if (CachedCameraEffectComponent)
		CachedCameraEffectComponent->PlayEffectPreset(CameraEffects);
}
