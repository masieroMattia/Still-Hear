
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_FootstepSound.generated.h"

UCLASS()
class STILLHEAR_API UAN_FootstepSound : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// Sound to play for grass surfaces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	USoundBase* GrassFootstepSound;
	// Sound to play for soil surfaces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	USoundBase* SoilFootstepSound;
	// Default footstep sound for other surfaces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	USoundBase* DefaultFootstepSound;
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
