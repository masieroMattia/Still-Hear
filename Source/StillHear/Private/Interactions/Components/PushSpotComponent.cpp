#include "Interactions/Components/PushSpotComponent.h"

#if WITH_EDITORONLY_DATA
#include "Components/ArrowComponent.h"
#endif

#pragma region CONSTRUCTOR
UPushSpotComponent::UPushSpotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
		
#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("SpotArrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor::Cyan;
		ArrowComponent->bTreatAsASprite = true; // This makes the arrow always face the camera in the editor, improving visibility
		ArrowComponent->SetupAttachment(this);
	}
#endif
}
#pragma endregion

#pragma region METHODS
FTransform UPushSpotComponent::GetLeftHandTransform() const
{
	const UActorComponent* MeshComponent = GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (MeshComponent)
	{
		const UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(MeshComponent);
		if (Mesh->DoesSocketExist(LeftHandSocketName))
			return Mesh->GetSocketTransform(LeftHandSocketName);
	}
    
	return GetComponentTransform(); // Fallback to the center of the spot if socket is missing
}

FTransform UPushSpotComponent::GetRightHandTransform() const
{
	const UActorComponent* MeshComponent = GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (MeshComponent)
	{
		const UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(MeshComponent);
		if (Mesh->DoesSocketExist(RightHandSocketName))
			return Mesh->GetSocketTransform(RightHandSocketName);
	}
	
	return GetComponentTransform(); // Fallback to the center of the spot if socket is missing
}
#pragma endregion