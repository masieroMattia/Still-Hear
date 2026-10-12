#include "EnemiesAI/Utility/Patrol/Waypoint.h"

AWaypoint::AWaypoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWaypoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!bSnapToFloor || !GetWorld())
		return;

	const FVector Origin = GetActorLocation();

	// Start well above so the trace works even if the waypoint is embedded in terrain
	const FVector TraceStart = Origin + FVector(0.f, 0.f, 10000.f);
	const FVector TraceEnd   = Origin - FVector(0.f, 0.f, 10000.f);

	FHitResult Hit;
	const FCollisionQueryParams Params(FName("WaypointSnapTrace"), false, this);
	
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (!bHit)
		return;

	// Place the pivot exactly at the surface hit point
	SetActorLocation(FVector(Origin.X, Origin.Y, Hit.ImpactPoint.Z + FloorOffset));
#endif
}

void AWaypoint::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(NextWaypoint))
	{
		if (bShowDebugTrace)
		{
			#if WITH_EDITOR
				DrawDebugLine(
					GetWorld(),
					GetActorLocation() + FVector(0, 0, 10),
					NextWaypoint->GetActorLocation() + FVector(0, 0, 10),
					FColor::Red,
					true,
					-1.0f,
					0,
					2
				);
			#endif
			
		}
	}
}

