#include "GameplayAbilitySystem/Cues/GC_MantisShiftBurst.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UGC_MantisShiftBurst::UGC_MantisShiftBurst()
{
}

bool UGC_MantisShiftBurst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// Simply call super, which calls SpawnBurstEffects
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}

void UGC_MantisShiftBurst::SpawnBurstEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return;

	// --- SFX Logic (Reused from parent) ---
	if (SFX)
	{
		USceneComponent* TargetSFXComponent = MyTarget->GetRootComponent();
		if (SpawnedOnCharacter)
		{
			if (const ACharacter* MyCharacter = Cast<ACharacter>(MyTarget))
			{
				if (SFXAttachPoint == EAttachPoint::MESH) 
					TargetSFXComponent = MyCharacter->GetMesh();
				else if (SFXAttachPoint == EAttachPoint::COMPONENT && SFXComponentTag != NAME_None)
				{
					TArray<USceneComponent*> Components;
					MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
					for (USceneComponent* Comp : Components)
					{
						if (Comp->ComponentHasTag(SFXComponentTag)) { TargetSFXComponent = Comp; break; }
					}
				}
			}
		}

		if (!Parameters.Location.IsNearlyZero())
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SFX, Parameters.Location);
		}
		else if (TargetSFXComponent)
		{
			UGameplayStatics::SpawnSoundAttached(SFX, TargetSFXComponent, SFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}
	}

	// --- VFX Logic (Custom with Parameters) ---
	if (VFX)
	{
		UNiagaraComponent* NiagaraComp;
		const bool bHasLocation = !Parameters.Location.IsNearlyZero();

		if (bHasLocation)
		{
			const FRotator SpawnRotation = Parameters.Normal.IsNearlyZero() ? MyTarget->GetActorRotation() : Parameters.Normal.Rotation();
			NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFX, Parameters.Location, SpawnRotation);
		}
		else
		{
			USceneComponent* TargetVFXComponent = MyTarget->GetRootComponent();
			if (SpawnedOnCharacter)
			{
				if (const ACharacter* MyCharacter = Cast<ACharacter>(MyTarget))
				{
					if (VFXAttachPoint == EAttachPoint::MESH) 
						TargetVFXComponent = MyCharacter->GetMesh();
					else if (VFXAttachPoint == EAttachPoint::COMPONENT && VFXComponentTag != NAME_None)
					{
						TArray<USceneComponent*> Components;
						MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
						for (USceneComponent* Comp : Components)
						{
							if (Comp->ComponentHasTag(VFXComponentTag)) { TargetVFXComponent = Comp; break; }
						}
					}
				}
			}
			NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, TargetVFXComponent, VFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}

		if (NiagaraComp)
		{
			// Set custom Mantis parameters
			const float Duration = Parameters.RawMagnitude;
			const bool bIsReappearing = Parameters.NormalizedMagnitude > 0.5f;

			NiagaraComp->SetFloatParameter(DurationParamName, Duration);
			NiagaraComp->SetBoolParameter(IsReappearingParamName, bIsReappearing);
		}
	}
}
