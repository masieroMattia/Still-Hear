#include "Animation/AnimNotifies/ANS_EnableHitBox.h"

#include "Components/ShapeComponent.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

FString UANS_EnableHitBox::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Enable HitBox [%s]"), *HitBoxTag.GetTagName().ToString());
}

void UANS_EnableHitBox::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!MeshComp)
		return;
	
	if (UShapeComponent* HitBox = FindHitBoxComponent(MeshComp->GetOwner(), HitBoxTag.GetTagName()))
		HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UANS_EnableHitBox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
		return;
	
	if (UShapeComponent* HitBox = FindHitBoxComponent(MeshComp->GetOwner(), HitBoxTag.GetTagName()))
		HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UShapeComponent* UANS_EnableHitBox::FindHitBoxComponent(const AActor* Owner, const FName& Tag)
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

