#include "Animation/AnimNotifies/AN_ReportNoiseEvent.h"

#include "GameFramework/Character.h"
#include "Perception/AISense_Hearing.h"

void UAN_ReportNoiseEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (Character)
		UAISense_Hearing::ReportNoiseEvent(
			MeshComp->GetWorld(),
			Character->GetActorLocation(),
			Loudness,
			Character,
			NoiseRange,
			Tag
		);
}
