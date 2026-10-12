#include "Character/Components/BlobShadowComponent.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#pragma region CONSTRUCTOR
UBlobShadowComponent::UBlobShadowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // Update after movement

	LeftShadowDecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("LeftShadowDecalComp"));
	
	// Rotate to point downwards (Decal X is the projection axis)
	LeftShadowDecalComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	
	// Default size
	LeftShadowDecalComp->DecalSize = FVector(ShadowProjectionDepth, ShadowBaseSize, ShadowBaseSize);

	LeftFoot.ShadowDecal = LeftShadowDecalComp;
	LeftFoot.SocketName = FName("foot_l");

	RightShadowDecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("RightShadowDecalComp"));
	
	// Rotate to point downwards (Decal X is the projection axis)
	RightShadowDecalComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	
	// Default size
	RightShadowDecalComp->DecalSize = FVector(ShadowProjectionDepth, ShadowBaseSize, ShadowBaseSize);

	RightFoot.ShadowDecal = RightShadowDecalComp;
	RightFoot.SocketName = FName("foot_r");
}
#pragma endregion

#pragma region METHODS
void UBlobShadowComponent::BeginPlay()
{
	Super::BeginPlay();

	// Establish instanced component references at runtime to bypass CDO serialization limitations in custom structs
	LeftFoot.ShadowDecal = LeftShadowDecalComp;
	RightFoot.ShadowDecal = RightShadowDecalComp;

	CachedOwner = GetOwner();
	if (const ACharacter* Character = Cast<ACharacter>(CachedOwner))
	{
		CachedMesh = Character->GetMesh();
	}
	else if (CachedOwner)
	{
		CachedMesh = CachedOwner->FindComponentByClass<USkeletalMeshComponent>();
	}

	InitializeFoot(LeftFoot, -1.0f);
	InitializeFoot(RightFoot, 1.0f);
}

void UBlobShadowComponent::OnRegister()
{
	Super::OnRegister();

	if (LeftShadowDecalComp)
	{
		LeftShadowDecalComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (RightShadowDecalComp)
	{
		RightShadowDecalComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UBlobShadowComponent::InitializeFoot(FBlobShadowFoot& Foot, const float SideMultiplier)
{
	const FVector ComponentLoc = GetComponentLocation();
	const FRotator ComponentRot = GetComponentRotation();

	if (CachedMesh && CachedMesh->DoesSocketExist(Foot.SocketName))
	{
		Foot.LastTrailSpawnLocation = CachedMesh->GetSocketLocation(Foot.SocketName);
	}
	else
	{
		Foot.LastTrailSpawnLocation = ComponentLoc + ComponentRot.RotateVector(FVector(0.0f, SideMultiplier * FootSpacing, 0.0f));
	}

	// Create dynamic material instances for runtime opacity control
	if (Foot.ShadowDecal && Foot.ShadowDecal->GetDecalMaterial())
	{
		Foot.SourceMaterial = Foot.ShadowDecal->GetDecalMaterial();
		Foot.DynamicMaterial = UMaterialInstanceDynamic::Create(Foot.SourceMaterial, this);
		Foot.ShadowDecal->SetDecalMaterial(Foot.DynamicMaterial);
		Foot.DynamicMaterial->SetScalarParameterValue(OpacityParameterName, 0.0f);
	}
}

void UBlobShadowComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateShadow(DeltaTime);
}

void UBlobShadowComponent::UpdateShadow(const float DeltaTime)
{
	UpdateFootShadow(LeftFoot, -1.0f, DeltaTime);
	UpdateFootShadow(RightFoot, 1.0f, DeltaTime);
}

void UBlobShadowComponent::UpdateFootShadow(FBlobShadowFoot& Foot, const float SideMultiplier, const float DeltaTime) const
{
	if (!Foot.ShadowDecal) 
		return;

	const FVector ComponentLoc = GetComponentLocation();
	const FRotator ComponentRot = GetComponentRotation();
	const float SafeFadeSpeed = ShadowFadeSpeed > 0.0f ? ShadowFadeSpeed : 10.0f;

	// Determine Foot Start location
	FVector StartLoc;
	if (CachedMesh && CachedMesh->DoesSocketExist(Foot.SocketName))
	{
		StartLoc = CachedMesh->GetSocketLocation(Foot.SocketName);
	}
	else
	{
		StartLoc = ComponentLoc + ComponentRot.RotateVector(FVector(0.0f, SideMultiplier * FootSpacing, 0.0f));
	}

	// Trace ground
	FHitResult GroundHit;
	const bool bTraceSuccess = TraceFootGround(StartLoc, GroundHit);

	if (bTraceSuccess && (ShadowMode == EBlobShadowMode::FootShadowsOnly || ShadowMode == EBlobShadowMode::Both))
	{
		Foot.ShadowOpacity = FMath::FInterpTo(Foot.ShadowOpacity, 1.0f, DeltaTime, SafeFadeSpeed);

		Foot.ShadowDecal->SetWorldLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, GroundOffset));
		const float TargetYaw = (CachedOwner ? CachedOwner->GetActorRotation().Yaw : 0.0f) + DecalRotationOffset;
		Foot.ShadowDecal->SetWorldRotation(FRotator(-90.0f, TargetYaw, 0.0f));
		Foot.ShadowDecal->SetVisibility(true);

		const float SizeRatio = FMath::Pow(FMath::Clamp(1.0f - (GroundHit.Distance / ShrinkDistance), 0.0f, 1.0f), ShrinkExponent);
		const float CurrentSize = ShadowBaseSize * FMath::Lerp(SizeFalloff, 1.0f, SizeRatio);
		Foot.ShadowDecal->DecalSize = FVector(ShadowProjectionDepth, CurrentSize * Foot.ShadowOpacity, CurrentSize * Foot.ShadowOpacity);
		Foot.ShadowDecal->MarkRenderStateDirty();
		Foot.ShadowDecal->FadeScreenSize = 0.0f;

		// Update opacity on the dynamic material
		if (Foot.DynamicMaterial)
		{
			Foot.DynamicMaterial->SetScalarParameterValue(OpacityParameterName, Foot.ShadowOpacity);
		}
	}
	else
	{
		Foot.ShadowOpacity = FMath::FInterpTo(Foot.ShadowOpacity, 0.0f, DeltaTime, SafeFadeSpeed);
		if (Foot.ShadowOpacity > 0.01f && (ShadowMode == EBlobShadowMode::FootShadowsOnly || ShadowMode == EBlobShadowMode::Both))
		{
			Foot.ShadowDecal->DecalSize = FVector(ShadowProjectionDepth, ShadowBaseSize * Foot.ShadowOpacity, ShadowBaseSize * Foot.ShadowOpacity);
			Foot.ShadowDecal->MarkRenderStateDirty();

			// Update opacity on the dynamic material
			if (Foot.DynamicMaterial)
			{
				Foot.DynamicMaterial->SetScalarParameterValue(OpacityParameterName, Foot.ShadowOpacity);
			}
		}
		else
		{
			Foot.ShadowDecal->SetVisibility(false);
			if (Foot.DynamicMaterial)
			{
				Foot.DynamicMaterial->SetScalarParameterValue(OpacityParameterName, 0.0f);
			}
		}
	}

	// Handle Trail Spawning
	if (bTraceSuccess && (ShadowMode == EBlobShadowMode::TrailOnly || ShadowMode == EBlobShadowMode::Both))
	{
		const float SizeRatio = FMath::Pow(FMath::Clamp(1.0f - (GroundHit.Distance / ShrinkDistance), 0.0f, 1.0f), ShrinkExponent);
		const float CurrentSize = ShadowBaseSize * FMath::Lerp(SizeFalloff, 1.0f, SizeRatio);

		const float DistanceMoved = FVector::Dist(StartLoc, Foot.LastTrailSpawnLocation);
		if (DistanceMoved >= Foot.TrailSpawnDistance)
		{
			SpawnTrailDecal(GroundHit, CurrentSize, Foot.TrailDecalSize, Foot.TrailLifespan, Foot.SourceMaterial);
			Foot.LastTrailSpawnLocation = StartLoc;
		}
	}
}

void UBlobShadowComponent::SpawnTrailDecal(const FHitResult& GroundHit, const float Size, const float TrailSize, const float Lifespan, UMaterialInterface* DecalMat) const
{
	if (!DecalMat) return;

	// Calculate the scale ratio relative to the main shadow size
	const float ScaleRatio = Size / ShadowBaseSize;
	const float FinalTrailSize = TrailSize * ScaleRatio;

	// Rotate to match the character's current yaw + offset
	const float TargetYaw = (CachedOwner ? CachedOwner->GetActorRotation().Yaw : 0.0f) + DecalRotationOffset;
	const FRotator DecalRotation = FRotator(-90.0f, TargetYaw, 0.0f);

	// Use SpawnDecalAtLocation for temporary trail decals
	UDecalComponent* TrailDecal = UGameplayStatics::SpawnDecalAtLocation(
		this,
		DecalMat,
		FVector(64.0f, FinalTrailSize, FinalTrailSize),
		GroundHit.ImpactPoint + FVector(0.0f, 0.0f, GroundOffset),
		DecalRotation,
		Lifespan
	);

	if (TrailDecal)
	{
		// Set it to fade out smoothly
		TrailDecal->SetFadeOut(Lifespan * 0.5f, Lifespan * 0.5f, false);
		TrailDecal->FadeScreenSize = 0.0f;
	}
}

bool UBlobShadowComponent::TraceFootGround(const FVector& FootLoc, FHitResult& OutHit) const
{
	// Start slightly above the foot to avoid missing the ground if the foot clips through
	const FVector Start = FootLoc + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = FootLoc - FVector(0.0f, 0.0f, MaxTraceDistance);

	FCollisionQueryParams Params;
	if (CachedOwner)
	{
		Params.AddIgnoredActor(CachedOwner);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, FloorTraceChannel, Params);
}
#pragma endregion
