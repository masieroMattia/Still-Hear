#include "Animation/AnimNotifies/AN_SendGameplayEventToSefl.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

FString UAN_SendGameplayEventToSelf::GetNotifyName_Implementation() const
{
	return "Send Gameplay Event";
}

// Sends a gameplay event to the owner when the animation notify is triggered
void UAN_SendGameplayEventToSelf::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp || !Animation)
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
		return;

	FGameplayEventData EventData;
	EventData.Instigator = Owner;	
	EventData.Target = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventTag,
		EventData
	);
}