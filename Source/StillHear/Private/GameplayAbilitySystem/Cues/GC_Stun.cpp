#include "GameplayAbilitySystem/Cues/GC_Stun.h"

#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnemiesAI/Utility/DataAssets/Abilities/StunData.h"

#pragma region CONSTRUCTOR
AGC_Stun::AGC_Stun()
{
	PrimaryActorTick.bCanEverTick = true;
}
#pragma endregion

#pragma region METHODS
bool AGC_Stun::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	CurrentTarget = MyTarget;
	if (!CurrentTarget) 
		return false;

	if (const ACharacter* Character = Cast<ACharacter>(CurrentTarget))
		CachedMesh = Character->GetMesh();

	bIsRemoving = false;
	CachedParameters = Parameters;
	TotalStunDuration = Parameters.RawMagnitude;
	ElapsedStunTime = 0.0f;

	if (StunData)
	{
		SetActorTickEnabled(true);
		SpawnedOnCharacter = true;
		VFXAttachPoint = EAttachPoint::ROOT;
	}

	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AGC_Stun::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGC_Stun::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRemoving && StunData && TotalStunDuration > 0.0f)
	{
		ElapsedStunTime += DeltaTime;
		const float Alpha = FMath::Clamp(ElapsedStunTime / TotalStunDuration, 0.0f, 1.0f);
		
		// RADIUS UPDATE
		if (StunData->RadiusGrowthCurve)
		{
			const float GrowthValue = StunData->RadiusGrowthCurve->GetFloatValue(ElapsedStunTime);
			
			// Shrink logic: start at 15%, finish at 95%
			constexpr float ShrinkStartAlpha = 0.15f;
			constexpr float ShrinkEndAlpha = 0.95f;
			
			float ShrinkFactor = 1.0f;
			if (Alpha > ShrinkStartAlpha)
			{
				ShrinkFactor = FMath::Clamp(1.0f - (Alpha - ShrinkStartAlpha) / (ShrinkEndAlpha - ShrinkStartAlpha), 0.0f, 1.0f);
			}
			
			const float FinalRadius = StunData->MaxRadius * GrowthValue * ShrinkFactor;
			if (SpawnedVFX)
			{
				SpawnedVFX->SetVariableFloat(StunData->RadiusParameterName, FinalRadius);
			}
		}

		// MATERIAL (DISSOLVE/FADE) UPDATE
		if (CachedMesh && StunData->DissolveCurve)
		{
			const float DissolveValue = StunData->DissolveCurve->GetFloatValue(ElapsedStunTime);
			
			// Return to 1.0 (Opaque) logic: start at 70%, finish at 95%
			constexpr float ReturnStartAlpha = 0.7f;
			constexpr float ReturnEndAlpha = 0.95f;
			
			float ReturnFactor = 0.0f;
			if (Alpha > ReturnStartAlpha)
			{
				ReturnFactor = FMath::Clamp((Alpha - ReturnStartAlpha) / (ReturnEndAlpha - ReturnStartAlpha), 0.0f, 1.0f);
			}
			
			const float FadeFactor = FMath::Lerp(DissolveValue, 1.0f, ReturnFactor);
			CachedMesh->SetScalarParameterValueOnMaterials(StunData->FadeOpacityParameterName, FadeFactor);
		}
	}
}
#pragma endregion
