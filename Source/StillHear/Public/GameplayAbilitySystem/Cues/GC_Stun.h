#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXActor.h"
#include "GC_Stun.generated.h"

class UStunData;

UCLASS()
class STILLHEAR_API AGC_Stun : public AGC_SoundAndVFXActor
{
	GENERATED_BODY()
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	TObjectPtr<UStunData> StunData;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> CachedMesh;

	FGameplayCueParameters CachedParameters;
	
	float TotalStunDuration = 0.0f;
	float ElapsedStunTime = 0.0f;
	bool bIsRemoving = false;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AGC_Stun();
#pragma endregion

#pragma region METHODS
protected:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void Tick(float DeltaTime) override;
#pragma endregion
};
