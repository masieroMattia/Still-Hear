
#include "Animation/AnimNotifies/AN_FootstepSound.h"

#include "Character/StillHearMainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodyInstance.h"
#include "TraceAndCollision/CustomSurface.h"

void UAN_FootstepSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(MeshComp->GetOwner());

	if (!Character)
		return;

	const EPhysicalSurface SurfaceType = Character->DetectFloorType();

	if (SurfaceType == ECustomSurface::Grass && GrassFootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(Character->GetWorld(), GrassFootstepSound, Character->GetActorLocation());
	}
	else if (SurfaceType == ECustomSurface::Soil && SoilFootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(Character->GetWorld(), SoilFootstepSound, Character->GetActorLocation());
	}
	else if (DefaultFootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), DefaultFootstepSound, Character->GetActorLocation());
	}
}
