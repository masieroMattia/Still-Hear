#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXActor.h"
#include "GC_MainCharacter_Death.generated.h"

UCLASS(Blueprintable, BlueprintType)
class STILLHEAR_API AGC_MainCharacter_Death : public AGC_SoundAndVFXActor
{
	GENERATED_BODY()

public:
	AGC_MainCharacter_Death();

protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

private:
	void HideTargetMesh(AActor* TargetActor);

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Death")
	float MeshHideDelay = 0.5f;

	FTimerHandle HideMeshTimerHandle;
};
