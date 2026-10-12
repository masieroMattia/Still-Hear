#pragma once

#include "CoreMinimal.h"
#include "EAttachPoint.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_CooldownFinishedBase.generated.h"

class UNiagaraSystem;
class USoundBase;

// Shared base for "cooldown finished" cues: stays silent on OnActive (added when the cooldown
// starts) and plays SFX/VFX on OnRemove (when the cooldown's GameplayEffect expires)
UCLASS(Abstract)
class STILLHEAR_API AGC_CooldownFinishedBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	AGC_CooldownFinishedBase();
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	bool SpawnedOnCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|VFX")
	TObjectPtr<UNiagaraSystem> VFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter == true", EditConditionHides))
	EAttachPoint VFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "VFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName VFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "VFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName VFXComponentTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter == true", EditConditionHides))
	EAttachPoint SFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName SFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName SFXComponentTag;
#pragma endregion

#pragma region METHODS
protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// Subclasses return the sound to play when the cooldown finishes (typically pulled from a data asset)
	virtual USoundBase* GetFinishCooldownSound() const { return nullptr; }
#pragma endregion
};
