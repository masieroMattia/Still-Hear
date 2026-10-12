#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrouchData.generated.h"

class UGameplayEffect;

UCLASS()
class STILLHEAR_API UCrouchData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Gameplay Effect applied while crouching
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> CrouchStatusEffect;

	// Duration of the cooldown in seconds
	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (ClampMin = "0.0"))
	float CooldownDuration = 0.3f;
};

