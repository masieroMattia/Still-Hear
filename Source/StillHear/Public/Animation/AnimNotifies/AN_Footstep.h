#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_Footstep.generated.h"

/**
 * Automatically detects the surface and plays the sound from the character's
 * FootStepData.
 */
UCLASS()
class STILLHEAR_API UAN_Footstep : public UAnimNotify {
  GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
	FName SocketName = NAME_None;

	virtual void Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference) override;
};
