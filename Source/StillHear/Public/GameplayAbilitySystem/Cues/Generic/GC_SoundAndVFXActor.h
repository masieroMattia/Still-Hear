#pragma once

#include "CoreMinimal.h"
#include "EAttachPoint.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_SoundAndVFXActor.generated.h"

UCLASS()
class STILLHEAR_API AGC_SoundAndVFXActor : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	bool SpawnedOnCharacter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX")
	TObjectPtr<class UNiagaraSystem> VFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter", EditConditionHides))
	EAttachPoint VFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter && VFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName VFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|VFX", meta = (EditCondition = "SpawnedOnCharacter && VFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName VFXComponentTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX")
	TObjectPtr<class USoundBase> SFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX")
	bool bLoop = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter", EditConditionHides))
	EAttachPoint SFXAttachPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter && SFXAttachPoint == EAttachPoint::MESH", EditConditionHides))
	FName SFXSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SFX", meta = (EditCondition = "SpawnedOnCharacter && SFXAttachPoint == EAttachPoint::COMPONENT", EditConditionHides))
	FName SFXComponentTag;

	// Reference to the spawned VFX and SFX so we can destroy and remove it at the end
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> SpawnedVFX;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> SpawnedSFX;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	AGC_SoundAndVFXActor();
#pragma endregion 

#pragma region METHODS
protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
#pragma endregion 
};
