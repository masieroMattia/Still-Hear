#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttackBaseData.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType)
class STILLHEAR_API UAttackBaseData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ===== HIT =====
	// GameplayEffect applied to the player on hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TSubclassOf<UGameplayEffect> AttackHitEffectClass;

	// Tag used to find the hit box component on the owning actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	FGameplayTag HitBoxTag;

	// ===== COOLDOWN =====
	// Duration in seconds of the attack cooldown
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float AttackCooldownDuration = 2.5f;
};

