#include "Animation/AnimNotifies/AN_EnableHitBox.h"

#include "Components/ShapeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAN_EnableHitBox::UAN_EnableHitBox()
{
	bEnableCollision = true;
}

FString UAN_EnableHitBox::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("%s HitBox [%s]"), bEnableCollision ? TEXT("Enable") : TEXT("Disable"), *HitBoxTag.GetTagName().ToString());
}

void UAN_EnableHitBox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
		return;

	if (UShapeComponent* HitBox = FindHitBoxComponent(MeshComp->GetOwner(), HitBoxTag.GetTagName()))
	{
		HitBox->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (bSendGameplayEvent && EventTag.IsValid())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
	}
}

UShapeComponent* UAN_EnableHitBox::FindHitBoxComponent(const AActor* Owner, const FName& Tag)
{
	if (!Owner || Tag.IsNone())
		return nullptr;

	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (Component && Component->IsA<UShapeComponent>() && Component->ComponentHasTag(Tag))
		{
			return Cast<UShapeComponent>(Component);
		}
	}

	return nullptr;
}
