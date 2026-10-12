#include "Interactions/Actors/Projectile.h"

#include "Interfaces/Targetable.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameplayTagAssetInterface.h"
#include "Components/AudioComponent.h"
#include "Perception/AISense_Hearing.h"
#include "TraceAndCollision/CustomCollision.h"
#include "Interactions/Components/TargetSpot.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"

#pragma region CONSTRUCTOR
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	
	SphereComponent->SetNotifyRigidBodyCollision(true);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECustomCollision::Companion, ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Ignore);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f; // Set to > 0 if you want bullet drop
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	
	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(RootComponent);
	
	ImpactNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactNiagaraComponent"));
	ImpactNiagaraComponent->SetupAttachment(RootComponent);
	ImpactNiagaraComponent->SetAutoActivate(false);
	
	DeflectionCount = 0;
}
#pragma endregion
	
#pragma region METHODS
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentSpeed = BaseSpeed; // Initialize the current speed to base speed
	
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->MaxSpeed = DeflectedSpeed;
		ProjectileMovementComponent->SetVelocityInLocalSpace(FVector::ForwardVector * CurrentSpeed);
	}
	
	SetLifeSpan(LifeSpan);
	
	SphereComponent->OnComponentHit.AddUniqueDynamic(this, &AProjectile::OnSphereHit);
	TrailAudioComponent = UGameplayStatics::SpawnSoundAttached(ProjectileTrailSound, RootComponent);
}

void AProjectile::LifeSpanExpired()
{
	StopAudioComponentAndDestroyActor(TrailAudioComponent);
}

void AProjectile::SetHomingTarget(USceneComponent* TargetComponent, const float InHomingAcceleration) const
{
	if (!TargetComponent || !ProjectileMovementComponent) 
		return;
    
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingAccelerationMagnitude = InHomingAcceleration;
	ProjectileMovementComponent->HomingTargetComponent = TargetComponent;
}

void AProjectile::StartMovingToSpot()
{
	if (!ProjectileMovementComponent || !SphereComponent) 
		return;
       
	// Wake up the movement component after the blocking hit stopped it
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->Velocity = DeflectionDirection * CurrentSpeed;
    
	GetWorldTimerManager().SetTimer(MoveToSpotTimerHandle, this, &AProjectile::PauseAtSpot, DeflectionDistanceTime, false);
}

void AProjectile::PauseAtSpot()
{
	// Stop the projectile exactly at the target spot
	ProjectileMovementComponent->StopMovementImmediately();
	bIsPaused = true;
    
	if (CurrentDeflector)
	{
		const UTargetSpot* TargetSpot = Cast<UTargetSpot>(CurrentDeflector->GetComponentByClass(UTargetSpot::StaticClass()));
		if (TargetSpot)
			SetActorLocation(TargetSpot->GetComponentLocation());
	}
	
	// Align rotation for the upcoming exit
	SetActorRotation(NextForwardVector.Rotation());
    
	// Start the pause timer before resuming movement
	GetWorldTimerManager().SetTimer(PauseTimerHandle, this, &AProjectile::ResumeMovement, DeflectionPauseDuration, false);
}

void AProjectile::ResumeMovement()
{
	bIsPaused = false;
    
	if (DeflectExitSound)
		UGameplayStatics::PlaySoundAtLocation(this, DeflectExitSound, GetActorLocation());
       
	// Call StopHit on the stored deflector
	if (CurrentDeflector)
	{
		ITargetable* TargetableInterface = Cast<ITargetable>(CurrentDeflector);
		if (TargetableInterface)
			TargetableInterface->StopHitTarget();
          
		CurrentDeflector = nullptr; // Reset the reference after using it
	}
	
	// Ensure movement is active
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->bIsHomingProjectile = false; 
	ProjectileMovementComponent->Velocity = NextForwardVector * CurrentSpeed;
    
	// Wait until the projectile has safely exited the deflector before enabling collision
	GetWorldTimerManager().SetTimer(EnableCollisionTimerHandle, this, &AProjectile::EnableCollision, TimeToExit, false);
}

void AProjectile::EnableCollision()
{
	if (SphereComponent)
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AProjectile::StopAudioComponentAndDestroyActor(const TObjectPtr<UAudioComponent> AudioComponent)
{
	if (AudioComponent)
		AudioComponent->Stop();
	
	Destroy();
}
#pragma endregion

#pragma region UFUNCTIONS
void AProjectile::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{	
	if (bIsPaused) 
		return;
	
	if (!OtherActor || OtherActor == this || !OtherComp)
	{
		StopAudioComponentAndDestroyActor(TrailAudioComponent);
		return;
	}

	const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
	const bool bCanDeflect = TagInterface && TagInterface->HasMatchingGameplayTag(TAG_Interact_Deflect);
	
	if (bCanDeflect)
	{
		CurrentSpeed = DeflectedSpeed;
		
		const UTargetSpot* TargetSpot = OtherActor->GetComponentByClass(UTargetSpot::StaticClass()) ? Cast<UTargetSpot>(OtherActor->GetComponentByClass(UTargetSpot::StaticClass())) : nullptr;
		if (TargetSpot)
		{
			if (DeflectEnterSoundSpawned)
			{
				BasePitchMultiplier += OffsetPitchMultiplier;
				DeflectEnterSoundSpawned->Stop();
				DeflectEnterSoundSpawned->SetPitchMultiplier(BasePitchMultiplier);
				DeflectEnterSoundSpawned->Play(0.0f);
			}
			else
				DeflectEnterSoundSpawned = UGameplayStatics::SpawnSoundAttached(DeflectEnterSound, RootComponent, NAME_None, GetActorLocation(), EAttachLocation::KeepWorldPosition, true, 1.0f, BasePitchMultiplier, 0.0f, nullptr, nullptr, false);
             
			// Store the deflector to interact with it later when resuming movement
			CurrentDeflector = OtherActor;

			if (!HitDeflectors.Contains(OtherActor))
			{
				DeflectionCount++;
				HitDeflectors.Add(OtherActor);
			}
			
			NextForwardVector = TargetSpot->GetForwardVector();
          
			// Prevent multiple collisions while moving to the center of the spot
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
          
			// Calculate the time needed to safely exit the deflector's bounds upon resume
			FVector Origin, Extent;
			OtherActor->GetActorBounds(false, Origin, Extent);
			const float ExitDistance = Extent.Size() + SphereComponent->GetScaledSphereRadius() + 10.0f;
			TimeToExit = ExitDistance / CurrentSpeed;
          
			const FVector SpotLocation = TargetSpot->GetComponentLocation();
			const FVector DirectionToSpot = SpotLocation - GetActorLocation();
			const float Distance = DirectionToSpot.Size();
			
			ITargetable* TargetableInterface = Cast<ITargetable>(CurrentDeflector);
			if (TargetableInterface)
				TargetableInterface->HitTarget(DeflectionCount);
			
			// Only move to the spot if it's a reasonable distance away, otherwise just pause at the current location
			if (Distance > 5.0f)
			{
				DeflectionDirection = DirectionToSpot.GetSafeNormal();
				DeflectionDistanceTime = Distance / CurrentSpeed;
             
				// Defer movement to next tick because the hit might not have fully resolved yet, which can cause the projectile to get stuck in the deflector
				GetWorldTimerManager().SetTimerForNextTick(this, &AProjectile::StartMovingToSpot);
			}
			else 
				PauseAtSpot();
		}
		return;
	}
	
	// Detach the Niagara component so it survives the actor's destruction
	ImpactNiagaraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	ImpactNiagaraComponent->Activate();
    
	// Report noise at the impact point, but with Instigator set to the player so AI investigates the origin point
	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		GetActorLocation(),
		NoiseLoudness,
		GetInstigator(),
		NoiseRange
	);
	
	// Check for Interaction
	ITargetable* TargetableInterface = Cast<ITargetable>(OtherActor);
	if (TargetableInterface) 
		TargetableInterface->HitTarget(DeflectionCount);
		
	// Spawn special VFX, play sound and let the AI react if hitting an AI enemy
	if (AStillHearAICharacterBase* HitAI = Cast<AStillHearAICharacterBase>(OtherActor))
	{
		HitAI->NotifyHitByProjectile(GetInstigator());

		if (EnemyHitVFX)
			UNiagaraFunctionLibrary::SpawnSystemAttached(EnemyHitVFX, OtherActor->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

		if (EnemyHitSound)
			UGameplayStatics::PlaySoundAtLocation(this, EnemyHitSound, GetActorLocation());
	}
	else if (!TargetableInterface)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ProjectileHitSound, GetActorLocation());
	}
	
	StopAudioComponentAndDestroyActor(TrailAudioComponent);
}
#pragma endregion