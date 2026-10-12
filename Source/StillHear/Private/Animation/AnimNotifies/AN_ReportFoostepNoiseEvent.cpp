#include "Animation/AnimNotifies/AN_ReportFoostepNoiseEvent.h"

#include "TraceAndCollision/CustomSurface.h"
#include "Character/StillHearMainCharacter.h"
#include "Animation/AnimInstances/MainCharacterAnimInstance.h"

void UAN_ReportFoostepNoiseEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(MeshComp->GetOwner());

	if (!Character)
		return;

	UMainCharacterAnimInstance* AnimInstance = Cast<UMainCharacterAnimInstance>(MeshComp->GetAnimInstance());

	if (!AnimInstance)
		return;

	if (Character->GetVelocity().Size2D() < MinSpeedThreshold)
		return;

	if (AnimInstance->bIsCrouching)
	{
		Tag = FName("Crouch");
	}
	else if (AnimInstance->bIsSprinting)
	{
		Tag = FName("Run");
	}
	else
	{
		Tag = FName("Walk");
	}
	
	const EPhysicalSurface SurfaceType = Character->DetectFloorType();
	
	if (SurfaceType == ECustomSurface::Soil)
	{
		Tag = FName(*(FName("Vibration.").ToString()) + Tag.ToString());
	}

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Tag.ToString());
	
	Super::Notify(MeshComp, Animation, EventReference);
}
