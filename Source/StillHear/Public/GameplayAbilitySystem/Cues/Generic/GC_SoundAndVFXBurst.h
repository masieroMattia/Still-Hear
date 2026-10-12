#pragma once

#include "CoreMinimal.h"
#include "EAttachPoint.h"
#include "GameplayCueNotify_Burst.h"
#include "GC_SoundAndVFXBurst.generated.h"

class USoundBase;
class UNiagaraSystem;

UCLASS()
class STILLHEAR_API UGC_SoundAndVFXBurst : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	UGC_SoundAndVFXBurst();
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	bool SpawnedOnCharacter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX")
	TObjectPtr<UNiagaraSystem> VFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter", EditConditionHides))
	EAttachPoint VFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter && VFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName VFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter && VFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName VFXComponentTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX")
	TObjectPtr<USoundBase> SFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter", EditConditionHides))
	EAttachPoint SFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter && SFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName SFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter && SFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName SFXComponentTag;
#pragma endregion

#pragma region METHODS
protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// Spawns SFX and VFX based on configuration. Can be overridden for custom logic
	virtual void SpawnBurstEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;
#pragma endregion
};