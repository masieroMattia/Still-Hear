#include "Interactions/Components/TargetSpot.h"

#include "DrawDebugHelpers.h"

#pragma region CONSTRUCTOR
UTargetSpot::UTargetSpot()
{
	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
#if WITH_EDITOR
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;
#endif
}
#pragma endregion 
	
#pragma region METHODS
#if WITH_EDITOR
void UTargetSpot::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bShowDebugLine && GetWorld() && !GetWorld()->IsGameWorld())
	{
		const FVector Start = GetComponentLocation();
		const FVector End = Start + GetForwardVector() * DebugLineLength;

		DrawDebugLine(GetWorld(), Start, End, DebugLineColor, false, -1.0f, 0, DebugLineThickness);
	}
}
#endif
#pragma endregion
