#include "Tools/TrajectoryPreview.h"

#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region CONSTRUCTOR
ATrajectoryPreview::ATrajectoryPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->ArrowColor = ArrowColor;
	ArrowComponent->ArrowSize = ArrowSize;
}
#pragma endregion 
	
#if WITH_EDITOR
#pragma region METHODS
void ATrajectoryPreview::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (GEngine && !ActorMovedDelegateHandle.IsValid()) // Ensure we only bind the delegate once to prevent multiple bindings if the actor is reconstructed multiple times
		ActorMovedDelegateHandle = GEngine->OnActorMoved().AddUObject(this, &ATrajectoryPreview::OnGlobalActorMoved);
	
	CachedProjectileRadius = 10.0f;
	if (SoundWaveData && SoundWaveData->ProjectileClass)
	{
		const AProjectile* Projectile = SoundWaveData->ProjectileClass->GetDefaultObject<AProjectile>();
		if (Projectile)
		{
			const USphereComponent* SphereComp = Projectile->FindComponentByClass<USphereComponent>();
			if (SphereComp)
				CachedProjectileRadius = SphereComp->GetScaledSphereRadius();
		}
	}
	
	DrawTrajectory();
}

void ATrajectoryPreview::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (GEngine && ActorMovedDelegateHandle.IsValid())
	{
		GEngine->OnActorMoved().Remove(ActorMovedDelegateHandle); // Unbind the delegate to prevent dangling references when the actor is destroyed
		ActorMovedDelegateHandle.Reset();
	}
}

void ATrajectoryPreview::OnGlobalActorMoved(AActor* MovedActor) const
{
	if (MovedActor == this)
		return;
	
	DrawTrajectory();
}

void ATrajectoryPreview::DrawTrajectory() const
{
	const UWorld* World = GetWorld();
	if (!World) 
		return;
	
	FlushPersistentDebugLines(World); // Clear previous debug lines

	FVector CurrentLocation = GetActorLocation();
	CurrentLocation += FVector(0.0f, 0.0f, LineTraceZOffset); // Add Z offset to the starting location of the line trace to ensure it starts above the ground
	FVector CurrentDirection = GetActorForwardVector();

	const FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(CachedProjectileRadius);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	for (int i = 0; i <= MaxBounces; ++i)
    {
        FVector EndLocation = CurrentLocation + (CurrentDirection * MaxDistance);
        FHitResult HitResult;
		
		const bool bHit = World->SweepSingleByChannel(
			HitResult,
			CurrentLocation,
			EndLocation,
			FQuat::Identity,
			ECC_WorldDynamic,
			CollisionSphere,
			QueryParams
		);

        if (bHit)
        {
        	// Draw the trajectory line and the impact point
            DrawDebugLine(World, CurrentLocation, HitResult.ImpactPoint, LineColor, true, -1.0f, 0, LineThickness);
            DrawDebugPoint(World, HitResult.ImpactPoint, ImpactPointSize, ImpactPointColor, true, -1.0f);

        	// Check if the hit component has the deflect tag, and if so, calculate the reflection vector and update the current location for the next iteration
            if (HitResult.GetComponent() && HitResult.GetComponent()->ComponentHasTag(TAG_Interact_Deflect.GetTag().GetTagName()))
            {
                CurrentDirection = FMath::GetReflectionVector(CurrentDirection, HitResult.ImpactNormal); // Calculate the reflection vector based on the current direction and the normal of the surface hit
                CurrentLocation = HitResult.ImpactPoint + (HitResult.ImpactNormal * (CachedProjectileRadius + 1.0f)); // Move the current location slightly away from the surface to prevent immediate re-collision
            }
            else
                break;
        }
        else // If no hit, draw the final trajectory line and exit the loop
        {
            DrawDebugLine(World, CurrentLocation, EndLocation, FColor::Green, true, -1.0f, 0, 4.0f);
            break;
        }
    }
}
#pragma endregion 

#endif