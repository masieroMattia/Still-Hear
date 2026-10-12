#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXActor.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AGC_SoundAndVFXActor::AGC_SoundAndVFXActor()
{
	VFX = nullptr;
	VFXAttachPoint = EAttachPoint::ROOT;
	VFXSocketName = NAME_None;
	VFXComponentTag = NAME_None;
	SpawnedVFX = nullptr;

	SFX = nullptr;
	SFXAttachPoint = EAttachPoint::ROOT;
	SFXSocketName = NAME_None;
	SFXComponentTag = NAME_None;
	SpawnedSFX = nullptr;

	bAutoDestroyOnRemove = true;
	bAllowMultipleOnActiveEvents = false;
}

bool AGC_SoundAndVFXActor::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);

	if (!MyTarget)
		return false;

	USceneComponent* TargetSFXComponent = MyTarget->GetRootComponent();
	USceneComponent* TargetVFXComponent = MyTarget->GetRootComponent();

	if (SpawnedOnCharacter)
	{
		ACharacter* MyCharacter = Cast<ACharacter>(MyTarget);
       
		// Handle SFX Component Selection
		if (SFXAttachPoint == EAttachPoint::MESH && MyCharacter)
			TargetSFXComponent = MyCharacter->GetMesh();
		else if (SFXAttachPoint == EAttachPoint::COMPONENT && SFXComponentTag != NAME_None)
		{
			TArray<USceneComponent*> Components;
			MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
			for (USceneComponent* Comp : Components)
			{
				if (Comp->ComponentHasTag(SFXComponentTag))
				{
					TargetSFXComponent = Comp;
					break;
				}
			}
		}

		// Handle VFX Component Selection
		if (VFXAttachPoint == EAttachPoint::MESH && MyCharacter)
			TargetVFXComponent = MyCharacter->GetMesh();
		else if (VFXAttachPoint == EAttachPoint::COMPONENT && VFXComponentTag != NAME_None)
		{
			TArray<USceneComponent*> Components;
			MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
			for (USceneComponent* Comp : Components)
			{
				if (Comp->ComponentHasTag(VFXComponentTag))
				{
					TargetVFXComponent = Comp;
					break;
				}
			}
		}
	}

	if (SFX && TargetSFXComponent)
		SpawnedSFX = UGameplayStatics::SpawnSoundAttached(SFX, TargetSFXComponent, SFXSocketName);

	if (VFX && TargetVFXComponent)
		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, TargetVFXComponent, VFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

	return true;
}

bool AGC_SoundAndVFXActor::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!SpawnedSFX && !SpawnedVFX)
		return false;

	if (SpawnedVFX)
	{
		SpawnedVFX->Deactivate(); // Deactivate the VFX
		SpawnedVFX = nullptr; // Clear the reference
	}

	if (SpawnedSFX && !SFX->IsOneShot())
	{
		SpawnedSFX->Stop();
		SpawnedSFX = nullptr;
	}
	
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

