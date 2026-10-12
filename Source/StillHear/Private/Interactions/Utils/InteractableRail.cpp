#include "Interactions/Utils/InteractableRail.h"
#pragma region CONSTRUCTOR
AInteractableRail::AInteractableRail()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SetRootComponent(SplineComponent);
}
#pragma endregion
	
#pragma region METHODS
USplineComponent* AInteractableRail::GetSpline() const
{
	return SplineComponent;
}

void AInteractableRail::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!SplineComponent || !bSnapToFloor)
		return;
	
	const int NumRailPoints = SplineComponent->GetNumberOfSplinePoints();

	const FCollisionQueryParams TraceParams(FName("RailFloorSnapTrace"), false, this);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	// For each point on the spline, perform a line trace downwards to find the floor and snap the point to it with the specified offset
	for (int i = 0; i < NumRailPoints; i++)
	{
		FVector PointLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector TraceStart = PointLocation + FVector(0.0f, 0.0f, 1000.0f);
		FVector TraceEnd = PointLocation - FVector(0.0f, 0.0f, 5000.0f);
		
		FHitResult HitResult;

		const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, TraceStart, TraceEnd, ObjectQueryParams, TraceParams);
		if (bHit)
		{
			FVector NewPointLocation = HitResult.Location + FVector(0.0f, 0.0f, FloorOffset);
			SplineComponent->SetLocationAtSplinePoint(i, NewPointLocation, ESplineCoordinateSpace::World);
		}
	}
}
#pragma endregion
