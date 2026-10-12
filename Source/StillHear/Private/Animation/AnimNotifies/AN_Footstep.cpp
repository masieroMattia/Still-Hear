#include "Animation/AnimNotifies/AN_Footstep.h"

#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Data/DataAssets/FootStepData.h"
#include "Character/StillHearMainCharacter.h"

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
		return;

	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	// Only fire on the character's primary mesh
	if (MeshComp != Character->GetMesh())
		return;

	// Detect the surface type
	const EPhysicalSurface SurfaceType = Character->DetectFloorType();

	// Determine play location (using socket if valid)
	const FVector PlayLocation = (SocketName != NAME_None) ? MeshComp->GetSocketLocation(SocketName) : Character->GetActorLocation();

	// Play sound & VFX if the config is found
	const UFootStepData* FootstepConfig = Character->GetFootstepConfig();
	if (FootstepConfig)
	{
		// Play sound
		USoundBase* SoundToPlay = FootstepConfig->GetSoundForSurface(SurfaceType);
		if (SoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(Character->GetWorld(), SoundToPlay, PlayLocation);
		}

		// Play Niagara VFX
		UNiagaraSystem* VFXToPlay = FootstepConfig->GetVFXForSurface(SurfaceType);
		if (VFXToPlay)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				Character->GetWorld(),
				VFXToPlay,
				PlayLocation,
				Character->GetActorRotation()
			);
		}
	}
}
