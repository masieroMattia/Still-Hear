#include "GameplayAbilitySystem/Cues/Generic/GC_CooldownFinishedBase.h"

#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#pragma region CONSTRUCTOR
AGC_CooldownFinishedBase::AGC_CooldownFinishedBase()
{
	bAutoDestroyOnRemove = true;
	SpawnedOnCharacter = true;

	VFXAttachPoint = EAttachPoint::ROOT;
	VFXSocketName = NAME_None;
	VFXComponentTag = NAME_None;

	SFXAttachPoint = EAttachPoint::ROOT;
	SFXSocketName = NAME_None;
	SFXComponentTag = NAME_None;
}
#pragma endregion

#pragma region METHODS
bool AGC_CooldownFinishedBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// Return true to acknowledge the cue is active, but do nothing so it waits silently in the background
	return true;
}

bool AGC_CooldownFinishedBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
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

	// Spawn the finish-cooldown sound provided by the subclass, attached to the chosen component
	if (USoundBase* FinishCooldownSound = GetFinishCooldownSound())
	{
		if (TargetSFXComponent)
			UGameplayStatics::SpawnSoundAttached(FinishCooldownSound, TargetSFXComponent, SFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}

	// Spawn the VFX attached to the chosen component
	if (VFX && TargetVFXComponent)
		UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, TargetVFXComponent, VFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
#pragma endregion
