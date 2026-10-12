#include "Interactions/Actors/PushObj.h"

#include "AbilitySystemGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Interfaces/IKTargetReceiver.h"
#include "UI/Indicator/IndicatorComponent.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Interactions/Components/PushSpotComponent.h"

#pragma region COSTRUCTOR
APushObj::APushObj()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Disable ticking by default, it will be enabled when the interaction starts and disabled when it ends
	
	Tags.Add(TAG_Climb.GetTag().GetTagName()); // Add the climb tag to this actor so it can be detected by the character's climbing ability
	Tags.Add(TAG_Counterweight.GetTag().GetTagName());
	
	PushBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PushBoxCollision"));
	SetRootComponent(PushBoxCollision);
	PushBoxCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    
	PushBoxCollision->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));

	if (StaticMeshComponent)
		StaticMeshComponent->SetupAttachment(PushBoxCollision);

	if (SphereComponent)
		SphereComponent->SetupAttachment(PushBoxCollision);

	if (InteractNiagaraComponent)
		InteractNiagaraComponent->SetupAttachment(PushBoxCollision);

	if (DefaultSceneRoot)
		DefaultSceneRoot->SetupAttachment(PushBoxCollision);
	
	PushSpotsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PushSpotsRoot"));
	PushSpotsRoot->SetupAttachment(RootComponent);
}
#pragma endregion

#pragma region METHODS
void APushObj::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Automatically adjust the Box Collision size to match the Static Mesh
	if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh() && PushBoxCollision)
	{
		// Get the unscaled bounding box of the mesh asset directly
		const FBox MeshBox = StaticMeshComponent->GetStaticMesh()->GetBoundingBox();
        
		// Multiply the extent by the Blueprint scale to get the real size
		const FVector ScaledExtent = MeshBox.GetExtent() * StaticMeshComponent->GetComponentScale();
		PushBoxCollision->SetBoxExtent(ScaledExtent);

		// Calculate the physical center of the mesh taking scale into account
		const FVector ScaledCenterOffset = MeshBox.GetCenter() * StaticMeshComponent->GetComponentScale();
        
		// Offset the mesh so its physical center aligns perfectly with the Box Root
		StaticMeshComponent->SetRelativeLocation(-ScaledCenterOffset);
	}
}

void APushObj::BeginPlay()
{
	Super::BeginPlay();
	
	GetComponents<UPushSpotComponent>(PushSpots);
}

void APushObj::Tick(const float DeltaTime)
{
	// Calculates how much to lift or lower the object in this frame
	CurrentLiftingOffset = FMath::FInterpTo(CurrentLiftingOffset, TargetLiftingOffset, DeltaTime, LiftingLerpSpeed);

	// Handles smooth lowering when no interactor is present
	if (!CurrentInteractor)
	{
		HandleLowering();
		return;
	}
    
	UpdateMovement();
	UpdateHandSlots();
	UpdateIK();
}

bool APushObj::TryFindGroundZ(const FVector& TraceLocation, const float CurrentZ, float& OutGroundZ) const
{
	constexpr float StepHeight = 45.0f;
	constexpr float TraceDepth = 500.0f;
	constexpr float BoxXYScale = 0.5f;
	constexpr float WalkableNormalZ = 0.5f;

	const FVector BoxExtent = PushBoxCollision->GetScaledBoxExtent();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(BoxExtent.X * BoxXYScale, BoxExtent.Y * BoxXYScale, BoxExtent.Z));
    
	const FVector TraceStart(TraceLocation.X, TraceLocation.Y, CurrentZ + StepHeight);
	const FVector TraceEnd(TraceLocation.X, TraceLocation.Y, CurrentZ - TraceDepth);
    
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (CurrentInteractor)
	{
		QueryParams.AddIgnoredActor(CurrentInteractor);
	}
    
	FHitResult GroundHit;
	if (GetWorld()->SweepSingleByChannel(GroundHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, BoxShape, QueryParams))
	{
		if (GroundHit.Normal.Z > WalkableNormalZ && !GroundHit.bStartPenetrating)
		{
			OutGroundZ = GroundHit.Location.Z;
			return true;
		}
	}
    
	return false;
}

void APushObj::HandleLowering()
{
	const FVector CurrentLoc = GetActorLocation();
	float GroundZ = CurrentLoc.Z - CurrentLiftingOffset;
    
	// Attempts to find the exact ground Z below the box
	TryFindGroundZ(CurrentLoc, CurrentLoc.Z - CurrentLiftingOffset, GroundZ);
    
	// Teleports smoothly downwards to the calculated exact Z position
	FVector TargetLoc = CurrentLoc;
	TargetLoc.Z = GroundZ + CurrentLiftingOffset;
    
	SetActorLocation(TargetLoc, false);
    
	if (FMath::IsNearlyZero(CurrentLiftingOffset, 0.1f))
	{
		SetActorTickEnabled(false);
	}
}

void APushObj::UpdateMovement()
{
	const FVector CurrentLocation = GetActorLocation();
    
	// Calculates base target location based on interactor position
	FVector TargetLocation = CurrentInteractor->GetActorLocation() + AttachmentOffset;
    
	// Snaps to ground to allow traversing slopes and uneven terrain independently of the interactor's Z coordinate
	float GroundZ = CurrentInteractor->GetActorLocation().Z + AttachmentOffset.Z;
	TryFindGroundZ(TargetLocation, CurrentLocation.Z, GroundZ);
    
	TargetLocation.Z = GroundZ + CurrentLiftingOffset;
    
	const FVector DeltaMove = TargetLocation - CurrentLocation;
	bool bActuallyMoved = false;
    
	// Moves the object, sweeping the BoxComponent to stop at obstacles
	if (!DeltaMove.IsNearlyZero())
	{
		FHitResult HitResult;
		AddActorWorldOffset(DeltaMove, true, &HitResult);
        
		// Slides along the surface instead of fully stopping, so a side wall doesn't block forward movement
		if (HitResult.bBlockingHit)
		{
			const FVector SlideVector = FVector::VectorPlaneProject(DeltaMove * (1.f - HitResult.Time), HitResult.Normal);
			AddActorWorldOffset(SlideVector, true, &HitResult);
		}

		const FVector ActualLocation = GetActorLocation();

		// Keeps the interactor aligned with the object: if the object couldn't fully reach its target (blocked by anything), snap the interactor back so it doesn't drift into/through the object
		constexpr float MovementTolerance = 0.5f;
		if (FVector::Dist2D(ActualLocation, TargetLocation) > MovementTolerance)
		{
			FVector CorrectedInteractorLoc = ActualLocation - AttachmentOffset;
            
			// Keeps the interactor's current Z coordinate, allowing gravity and ground collision to handle its vertical position
			CorrectedInteractorLoc.Z = CurrentInteractor->GetActorLocation().Z;
            
			CurrentInteractor->SetActorLocation(CorrectedInteractorLoc, false);
		}
        
		if (!ActualLocation.Equals(CurrentLocation, 0.1f))
		{
			bActuallyMoved = true;
		}
	}
    
	UpdateAudio(bActuallyMoved);
}

void APushObj::UpdateAudio(bool bActuallyMoved)
{
	if (bActuallyMoved != bWasMovingLastFrame)
	{
		if (bActuallyMoved)
		{
			if (!IsValid(SpawnedAudio))
			{
				if (DragInteractionSound)
				{
					SpawnedAudio = UGameplayStatics::SpawnSoundAttached(DragInteractionSound, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1.0f);
				}
			}
			
			if (IsValid(SpawnedAudio))
			{
				if (SpawnedAudio->bIsPaused)
				{
					SpawnedAudio->SetPaused(false);
				}
				if (FadeInDuration > 0.0f)
				{
					SpawnedAudio->FadeIn(FadeInDuration, 1.0f);
				}
				else
				{
					SpawnedAudio->Play();
				}
			}
		}
		else
		{
			if (IsValid(SpawnedAudio))
			{
				if (FadeOutDuration > 0.0f)
				{
					SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
				}
				else
				{
					SpawnedAudio->Stop();
				}
			}
		}
		bWasMovingLastFrame = bActuallyMoved;
	}
}

void APushObj::UpdateHandSlots() const
{
	if (!PushSpotsRoot || !CurrentInteractor)
	{
		return;
	}
    
	// Keeps hand slots at the same height relative to the player's body
	const float TargetWorldZ = CurrentInteractor->GetActorLocation().Z + InteractorToSpotsZ + CurrentLiftingOffset;
	const float DesiredRelativeZ = TargetWorldZ - GetActorLocation().Z;

	constexpr float MaxSlideScale = 0.8f;
	const float MaxSlide = PushBoxCollision->GetScaledBoxExtent().Z * MaxSlideScale;
	const float ClampedRelativeZ = FMath::Clamp(DesiredRelativeZ, InitialPushSpotsRootRelativeZ - MaxSlide, InitialPushSpotsRootRelativeZ + MaxSlide);
    
	FVector NewRelLoc = PushSpotsRoot->GetRelativeLocation();
	NewRelLoc.Z = ClampedRelativeZ;
	PushSpotsRoot->SetRelativeLocation(NewRelLoc);
}

void APushObj::UpdateIK() const
{
	if (!ActivePushSpot || !CurrentInteractorMesh)
	{
		return;
	}
       
	IIKTargetReceiver* IKReceiver = Cast<IIKTargetReceiver>(CurrentInteractorMesh->GetAnimInstance());
	if (!IKReceiver)
	{
		return;
	}
       
	const FTransform LeftHandTransform = ActivePushSpot->GetLeftHandTransform();
	const FTransform RightHandTransform = ActivePushSpot->GetRightHandTransform();
    
	IKReceiver->UpdateIKTargets(LeftHandTransform, RightHandTransform);
}


void APushObj::ApplySpeedMultiplierToOwner(const TObjectPtr<ACharacter> & Interactor, const float Multiplier)
{
	UAbilitySystemComponent* InteractorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor);
	if (!InteractorASC)
		return;
	
	// Safely remove the exact previous effect using its unique handle
	if (CurrentSpeedEffectHandle.IsValid())
	{
		InteractorASC->RemoveActiveGameplayEffect(CurrentSpeedEffectHandle);
		CurrentSpeedEffectHandle.Invalidate();
	}
	
	const FGameplayEffectSpecHandle SpecHandle = InteractorASC->MakeOutgoingSpec(
		DragEffectClass,
		1.0f,
		InteractorASC->MakeEffectContext()
	);

	if (!SpecHandle.IsValid())
		return;

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")),
		Multiplier
	);

	// Apply the effect to change the values in the character class
	CurrentSpeedEffectHandle = InteractorASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

UPushSpotComponent* APushObj::GetNearestPushSpot(const FVector& FromLocation) const
{
	if (PushSpots.IsEmpty())
		return nullptr;
    
	UPushSpotComponent* BestSpot = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
    
	for (const TObjectPtr<UPushSpotComponent>& Spot : PushSpots)
	{
		if (!Spot)
			continue;

		const float DistSq = FVector::DistSquared(FromLocation, Spot->GetComponentLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestSpot = Spot;
		}
	}

	return BestSpot;
}
#pragma endregion
	
#pragma region INTERFACE METHODS
void APushObj::StartInteraction(TObjectPtr<ACharacter> Interactor)
{
	if (Interactor)
	{
		// Cache the attachment offset immediately
		AttachmentOffset = GetActorLocation() - Interactor->GetActorLocation();
	}
	
	Super::StartInteraction(Interactor);
}

void APushObj::ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor)
{
	Super::ExecuteStartInteraction(Interactor);
	
	// Change the displayed prompts during the interaction and re-register the indicator
	if (IndicatorComponent && PushingPromptActions.Num() > 0)
	{
		IndicatorComponent->UpdatePromptActions(PushingPromptActions, PushingPromptSeparatorClass);
		IndicatorComponent->RegisterIndicator();
	}
    
    if (!Interactor)
        return;
    
    CurrentInteractor = Interactor;
    CurrentInteractorMesh = CurrentInteractor->GetMesh();    
    
    if (!CurrentInteractorMesh)
        return;
    
    CurrentInteractorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Interactor);
    
    if (!CurrentInteractorASC)
        return;
    
    CurrentInteractorMovementComponent = Interactor->GetCharacterMovement();
    
    if (!CurrentInteractorMovementComponent)
        return;
    
    ActivePushSpot = GetNearestPushSpot(CurrentInteractor->GetActorLocation());

    if (ActivePushSpot)
    {
        // If the player couldn't reach the ideal spot (e.g. squeezed against a nearby wall), shift the object
        // forward by the same amount so the player has room and the hand IK targets stay aligned
        FVector PushDelta = CurrentInteractor->GetActorLocation() - ActivePushSpot->GetComponentLocation();
        PushDelta.Z = 0.0f;

        if (!PushDelta.IsNearlyZero())
        {
            AddActorWorldOffset(PushDelta, true);
            AttachmentOffset = GetActorLocation() - CurrentInteractor->GetActorLocation();
        }
    }

    if (PushSpotsRoot)
    {
        InitialPushSpotsRootRelativeZ = PushSpotsRoot->GetRelativeLocation().Z;
        InteractorToSpotsZ = PushSpotsRoot->GetComponentLocation().Z - CurrentInteractor->GetActorLocation().Z;
    }

    // Ignore the interactor's collision for both the Box and the Mesh
    if (PushBoxCollision)
        PushBoxCollision->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Ignore);
    
    if (StaticMeshComponent)
        StaticMeshComponent->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Ignore);

    ApplySpeedMultiplierToOwner(CurrentInteractor, DragMultiplier);

    bWasMovingLastFrame = false;
    
    TargetLiftingOffset = LiftingZOffset;

    SetActorTickEnabled(true);
}

void APushObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	if (IndicatorComponent)
	{
		IndicatorComponent->UpdatePromptActions(OriginalPromptActions, OriginalSeparatorClass);
	}

	CurrentInteractor = nullptr;
    
	if (CurrentInteractorASC && CurrentSpeedEffectHandle.IsValid())
	{
		CurrentInteractorASC->RemoveActiveGameplayEffect(CurrentSpeedEffectHandle);
		CurrentSpeedEffectHandle.Invalidate();
	}
    
	if (IsValid(SpawnedAudio))
	{
		if (FadeOutDuration > 0.0f)
		{
			SpawnedAudio->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			SpawnedAudio->Stop();
		}
		SpawnedAudio = nullptr;
	}

	CurrentInteractorASC = nullptr;
	CurrentInteractorMovementComponent = nullptr;
    
	TargetLiftingOffset = 0.0f;
	
    if (PushSpotsRoot)
    {
        FVector ResetLoc = PushSpotsRoot->GetRelativeLocation();
        ResetLoc.Z = InitialPushSpotsRootRelativeZ;
        PushSpotsRoot->SetRelativeLocation(ResetLoc);
    }

	// Restore collision
	if (PushBoxCollision)
		PushBoxCollision->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Block);
        
	if (StaticMeshComponent)
		StaticMeshComponent->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Block);

	Super::EndInteraction(Interactor);
}

void APushObj::Reset()
{
	Super::Reset();
}

bool APushObj::GetNearestInteractionSpotLocation(const FVector& FromLocation, FVector& OutLocation, FVector& OutDirection)
{
	const UPushSpotComponent* NearestSpot = GetNearestPushSpot(FromLocation);
	if (!NearestSpot)
		return false;

	OutLocation = NearestSpot->GetComponentLocation();
	OutDirection = NearestSpot->GetForwardVector();
    
	return true;
}
#pragma endregion
