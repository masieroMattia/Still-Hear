#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Chaos/ChaosEngineInterface.h"
#include "FootStepData.generated.h"

class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct FFootstepEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	TObjectPtr<UNiagaraSystem> VFX = nullptr;
};

/**
 * Data Asset that maps Physical Surfaces to Footstep Sounds and Niagara VFX.
 */
UCLASS(BlueprintType)
class STILLHEAR_API UFootStepData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Map to link a Physical Surface to its corresponding sound and VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	TMap<TEnumAsByte<EPhysicalSurface>, FFootstepEffects> SurfaceEffectsMap;

	/** Fallback effects if the surface is not found in the map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	FFootstepEffects DefaultEffects;

	/** Helper to get sound for a specific surface */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Audio")
	USoundBase* GetSoundForSurface(EPhysicalSurface SurfaceType) const;

	/** Helper to get Niagara system for a specific surface */
	UFUNCTION(BlueprintCallable, Category = "Footstep|VFX")
	UNiagaraSystem* GetVFXForSurface(EPhysicalSurface SurfaceType) const;
};
