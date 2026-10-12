#include "VFX/TargetMarkerNiagaraComponent.h"

#pragma region CONSTRUCTOR
UTargetMarkerNiagaraComponent::UTargetMarkerNiagaraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
#pragma endregion
	
#pragma region METHODS
void UTargetMarkerNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateNiagaraVariables();
	UpdateMarkerPosition();
}

void UTargetMarkerNiagaraComponent::UpdateNiagaraVariables()
{
	SetVariableLinearColor(COLOR, Color); 
	SetVariableFloat(BASE_SIZE, BaseSize);
	SetVariableFloat(MARKER_SIZE, MarkerSize);
}

void UTargetMarkerNiagaraComponent::UpdateMarkerPosition()
{
	if (!GetOwner()) return;

	FVector Origin;
	FVector BoxExtent;
    
	// Get the bounding box of the owning actor to find its physical height
	GetOwner()->GetActorBounds(false, Origin, BoxExtent);

	// Calculate the Z coordinate for the top of the actor, plus our custom offset
	const float TopZ = Origin.Z + BoxExtent.Z + MarkerZOffset;
    
	// Calculate the relative Z difference from the actor's pivot to the top
	const float RelativeZOffset = TopZ - GetOwner()->GetActorLocation().Z;
    
	// Apply the offset
	SetVariableVec3(MARKER_OFFSET, FVector(0.0f, 0.0f, RelativeZOffset));
	SetVariableVec3(BASE_OFFSET, FVector(0.0f, 0.0f, BaseZOffset));
}
#pragma endregion
