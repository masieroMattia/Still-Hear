// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AN_DetectFloorType.h"

#include "Character/StillHearMainCharacter.h"
#include "TraceAndCollision/CustomSurface.h"
#include "TraceAndCollision/FloorTypeEnum.h"

void UAN_DetectFloorType::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// OBSOLETE: This notify is now handled by UAN_Footstep.
	// We keep the function empty to avoid breaking existing animations until they are migrated.
	Super::Notify(MeshComp, Animation, EventReference);
}
