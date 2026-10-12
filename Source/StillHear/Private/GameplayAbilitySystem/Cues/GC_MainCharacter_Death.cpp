#include "GameplayAbilitySystem/Cues/GC_MainCharacter_Death.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

AGC_MainCharacter_Death::AGC_MainCharacter_Death()
{
	GameplayCueTag = TAG_GameplayCue_MainCharacter_Death;
}

bool AGC_MainCharacter_Death::OnActive_Implementation(
	AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	bool bResult = Super::OnActive_Implementation(MyTarget, Parameters);

	if (bResult && MyTarget)
	{
		TWeakObjectPtr<AActor> WeakTarget = MyTarget;
		GetWorldTimerManager().SetTimer(
			HideMeshTimerHandle,
			[this, WeakTarget]()
			{
				if (WeakTarget.IsValid())
				{
					HideTargetMesh(WeakTarget.Get());
				}
			},
			MeshHideDelay, false);
	}

	return bResult;
}

bool AGC_MainCharacter_Death::OnRemove_Implementation(
	AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	GetWorldTimerManager().ClearTimer(HideMeshTimerHandle);

	if (ACharacter* TargetCharacter = Cast<ACharacter>(MyTarget))
	{
		if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
		{
			Mesh->SetHiddenInGame(false);
		}
	}

	Super::OnRemove_Implementation(MyTarget, Parameters);

	return true;
}

void AGC_MainCharacter_Death::HideTargetMesh(AActor* TargetActor)
{
	if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
		{
			Mesh->SetHiddenInGame(true);
		}
	}
}
