#include "Character/FloatingCompanionComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "Components/SplineComponent.h"
#include "Components/PointLightComponent.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "Interactions/Components/CompanionInteractionSpotComponent.h"

UFloatingCompanionComponent::UFloatingCompanionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CompanionMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CompanionMesh"));
    CompanionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CompanionMesh->SetSimulatePhysics(false);
    CompanionMesh->SetGenerateOverlapEvents(false);
    
    // Create the point light
    CompanionLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CompanionLight"));
    
    CompanionLight->Intensity = 50.f;
    CompanionLight->AttenuationRadius = 500.f;

    // Disable Inverse Squared Falloff and set the custom exponent
    CompanionLight->bUseInverseSquaredFalloff = false;
    CompanionLight->LightFalloffExponent = 2.0f;
}

// ─────────────────────────────────────────────
//  BeginPlay
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure subobjects are valid and not pointing to CDO
    if (CompanionMesh && CompanionMesh->IsTemplate())
    {
        if (USkeletalMeshComponent* FoundMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetDefaultSubobjectByName(TEXT("CompanionMesh"))))
        {
            CompanionMesh = FoundMesh;
        }
    }
    
    if (CompanionLight && CompanionLight->IsTemplate())
    {
        if (UPointLightComponent* FoundLight = Cast<UPointLightComponent>(GetOwner()->GetDefaultSubobjectByName(TEXT("CompanionLight"))))
        {
            CompanionLight = FoundLight;
        }
    }

    SetupCompanionMesh();

    if (CompanionLight)
    {
        if (CompanionMesh)
        {
            CompanionLight->AttachToComponent(CompanionMesh, FAttachmentTransformRules::KeepRelativeTransform);
        }
        
        CompanionLight->SetVisibility(bIsLightEnabled);
    }
    
    // Initial position
    if (const AActor* Owner = GetOwner())
    {
        CurrentCompanionLocation = Owner->GetActorLocation() + BaseOffset;
        LastProgressLocation = CurrentCompanionLocation;
    }

    // Initial Config
    ActiveConfig = GetConfigForState(CurrentState);
    PreviousConfig = ActiveConfig;
    CurrentMorphName = ActiveConfig.MorphTargetName;
    CurrentColor = ActiveConfig.Color;
    CurrentMeshScale = ActiveConfig.MeshScale;
    
    // Start custom tick for obstacle checking
    if (bPreventClipping)
    {
        GetWorld()->GetTimerManager().SetTimer(ObstacleCheckTimerHandle, this, &UFloatingCompanionComponent::PerformObstacleCheck, ObstacleCheckInterval, true);
    }

    SetCompanionState(ECompanionState::Happy);
}

void UFloatingCompanionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (const UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ObstacleCheckTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}


// ─────────────────────────────────────────────
//  Setup mesh and dynamic material
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::SetupCompanionMesh()
{
    if (!CompanionMesh) return;
    

    // Dynamic setup of the mesh asset if configured
    if (CompanionMeshAsset && CompanionMesh)
    {
        CompanionMesh->SetSkeletalMesh(CompanionMeshAsset);
    }

    // Create a Dynamic Material Instance based on first material
    if (CompanionMesh)
    {
        if (UMaterialInterface* Mat = CompanionMesh->GetMaterial(0))
        {
            DynMaterial = CompanionMesh->CreateDynamicMaterialInstance(0, Mat);
        }
    }
}

// ─────────────────────────────────────────────
//  Tick
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoPollingEnabled)
    {
        PollEnvironment(DeltaTime);
    }

    UpdateFloating(DeltaTime);
    UpdateMorphTarget(DeltaTime);
    UpdateVisuals(DeltaTime);
}

// ─────────────────────────────────────────────
//  Floating: orbital position + bob
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::UpdateFloating(const float DeltaTime)
{
    if (!CompanionMesh) return;

    const AActor* Owner = GetOwner();
    FVector TargetLocation;

    if (CurrentInteractionSpot)
    {
        InteractionTimer += DeltaTime;
        
        switch (CurrentInteractionSpot->BehaviorType)
        {
            case ECompanionBehavior::AttachToSpot:
            {
                TargetLocation = CurrentInteractionSpot->GetComponentLocation();
                break;
            }
            case ECompanionBehavior::OrbitAround:
            {
                InteractionOrbitAngle += CurrentInteractionSpot->OrbitSpeed * DeltaTime;
                if (InteractionOrbitAngle > 360.f) 
                    InteractionOrbitAngle -= 360.f;
                
                const float TotalAngle = FMath::DegreesToRadians(InteractionOrbitAngle);
                const FVector OrbitOffset(
                    FMath::Cos(TotalAngle) * CurrentInteractionSpot->OrbitRadius,
                    FMath::Sin(TotalAngle) * CurrentInteractionSpot->OrbitRadius,
                    0.f);
                
                TargetLocation = CurrentInteractionSpot->GetComponentLocation() + OrbitOffset;
                break;
            }
            case ECompanionBehavior::FollowSpline:
            {
                if (CachedInteractionSpline)
                {
                    const USplineComponent* Spline = CachedInteractionSpline;
                    float SplineDuration = 1.0f;
                    float CurveEvalValue;

                    if (CurrentInteractionSpot->SplineProgressCurve)
                    {
                        float MinTime, MaxTime;
                        CurrentInteractionSpot->SplineProgressCurve->GetTimeRange(MinTime, MaxTime);
                        SplineDuration = FMath::Max(0.01f, MaxTime);
                    }
                    
                    const float RawAlpha = InteractionTimer / SplineDuration;
                    
                    // Handle ping-pong and looping behavior
                    if (CurrentInteractionSpot->bPingPongSpline)
                    {
                        const float tMod = FMath::Fmod(RawAlpha, 2.0f);
                        const float CurrentAlpha = (tMod > 1.0f) ? (2.0f - tMod) : tMod;
                        CurveEvalValue = CurrentAlpha * SplineDuration;
                    }
                    else if (Spline->IsClosedLoop())
                    {
                        CurveEvalValue = FMath::Fmod(InteractionTimer, SplineDuration);
                    }
                    else
                    {
                        CurveEvalValue = FMath::Clamp(InteractionTimer, 0.0f, SplineDuration);
                        
                        if (InteractionTimer >= SplineDuration && bIsFinishingSpline)
                        {
                            ClearInteractionSpot();
                            
                            // Immediately switch to follow-owner behavior for this frame
                            if (Owner)
                            {
                                TargetLocation = ComputeTargetLocation();
                            }
                            break;
                        }
                    }
                    
                    if (CurrentInteractionSpot) // Ensure it wasn't cleared
                    {
                        float Alpha;
                        if (CurrentInteractionSpot->SplineProgressCurve)
                        {
                            Alpha = CurrentInteractionSpot->SplineProgressCurve->GetFloatValue(CurveEvalValue);
                        }
                        else
                        {
                            Alpha = CurveEvalValue / SplineDuration;
                        }
                        
                        const float Distance = Alpha * Spline->GetSplineLength();
                        TargetLocation = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
                        
                    }
                }
                else
                {
                    TargetLocation = CurrentInteractionSpot->GetComponentLocation(); // Fallback
                    if (bIsFinishingSpline) ClearInteractionSpot();
                }
                break;
            }
        }
    }
    else
    {
        if (!Owner) 
            return;

        TimeAccumulator += DeltaTime;
        // Wrap to avoid precision issues over very long sessions
        if (TimeAccumulator > 1000.0f) TimeAccumulator -= 1000.0f;
        
        // Orbit
        CurrentOrbitAngle += ActiveConfig.OrbitSpeed * DeltaTime;
        if (CurrentOrbitAngle > 360.f) CurrentOrbitAngle -= 360.f;

        TargetLocation = ComputeTargetLocation();
    }

    // ── Follow speed (with catch-up boost when lagging behind) ───────────
    float CurrentFollowSpeed = CurrentInteractionSpot
        ? CurrentInteractionSpot->InteractionFollowSpeed
        : FollowSpeed;

    // If the companion is very far from the owner, boost the follow speed
    if (!CurrentInteractionSpot && Owner)
    {
        const float DistToOwner = FVector::Dist(CurrentCompanionLocation, Owner->GetActorLocation());
        if (DistToOwner > MaxLagDistance)
        {
            // Progressively faster the further away we are
            const float BoostFactor = FMath::Clamp((DistToOwner - MaxLagDistance) / MaxLagDistance, 0.f, 3.f);
            CurrentFollowSpeed *= (1.f + BoostFactor * 2.f);
        }
    }

    // Calculate where we want to move to
    const FVector NextPos = FMath::VInterpTo(CurrentCompanionLocation, TargetLocation, DeltaTime, CurrentFollowSpeed);

    // ── Obstacle avoidance (disabled during interactions) ────────────────
    if (bPreventClipping && !CurrentInteractionSpot)
    {
        // Smoothly blend toward the avoidance target instead of snapping
        if (bIsAvoiding)
        {
            SmoothedAvoidanceTarget = FMath::VInterpTo(SmoothedAvoidanceTarget, IdealTargetLocation, DeltaTime, AvoidanceSmoothSpeed);
            CurrentCompanionLocation = SmoothedAvoidanceTarget;
        }
        else
        {
            CurrentCompanionLocation = NextPos;
            SmoothedAvoidanceTarget = CurrentCompanionLocation;
        }

        // ── Stuck detection ──────────────────────────────────────────
        const float ProgressMadeSq = FVector::DistSquared(CurrentCompanionLocation, LastProgressLocation);
        const float DistToTargetSq = FVector::DistSquared(CurrentCompanionLocation, TargetLocation);

        // Frame-rate independent stuck detection: check progress speed
        const float CurrentSpeedSq = ProgressMadeSq / FMath::Max(DeltaTime * DeltaTime, 1.e-6f);
        if (DistToTargetSq > FMath::Square(StuckDistanceThreshold) && CurrentSpeedSq < FMath::Square(10.0f)) // 10 units/sec
        {
            StuckTimer += DeltaTime;
            if (StuckTimer >= StuckTeleportTime)
            {
                CurrentCompanionLocation = FMath::VInterpTo(CurrentCompanionLocation, TargetLocation, DeltaTime, 20.f);
                StuckTimer = 0.f;
                bIsAvoiding = false;
            }
        }
        else
        {
            StuckTimer = 0.f;
        }

        LastProgressLocation = CurrentCompanionLocation;
    }
    else
    {
        CurrentCompanionLocation = NextPos;
    }

    CompanionMesh->SetWorldLocation(CurrentCompanionLocation, false);
}

void UFloatingCompanionComponent::PerformObstacleCheck()
{
    if (!bPreventClipping || CurrentInteractionSpot)
    {
        bIsAvoiding = false;
        return;
    }

    const AActor* Owner = GetOwner();
    if (!Owner) 
        return;

    const FVector DesiredTarget = ComputeTargetLocation();
    IdealTargetLocation = SolveObstacleAvoidance(CurrentCompanionLocation, DesiredTarget, Owner);

    bIsAvoiding = !IdealTargetLocation.Equals(DesiredTarget, 1.0f);
}

// ─────────────────────────────────────────────
//  Obstacle Avoidance: gentle deflection and vertical nudge
// ─────────────────────────────────────────────
FVector UFloatingCompanionComponent::SolveObstacleAvoidance(const FVector& Current, const FVector& Desired, const AActor* IgnoreActor) const
{
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(IgnoreActor);
    if (CompanionMesh)
    {
        if (const AActor* CompOwner = CompanionMesh->GetOwner())
            Params.AddIgnoredActor(CompOwner);
    }

    const FCollisionShape Sphere = FCollisionShape::MakeSphere(CollisionSphereRadius);
    FHitResult Hit;

    // Sweep from the Owner's center to the desired companion position
    // This ensures the companion always stays on the player's side of the wall and never shoots through it
    const FVector Start = IgnoreActor ? IgnoreActor->GetActorLocation() : Current;

    if (!GetWorld()->SweepSingleByChannel(Hit, Start, Desired, FQuat::Identity, ECC_Visibility, Sphere, Params))
    {
        // No obstruction
        return Desired;
    }

    // If obstructed, try a "Vertical Escape": check if we can float higher to pass over the obstacle
    FVector VerticalDesired = Desired + FVector(0, 0, VerticalEscapeHeight);
    FHitResult VerticalHit;
    
    // Check if the path to the higher position is clear
    if (!GetWorld()->SweepSingleByChannel(VerticalHit, Start, VerticalDesired, FQuat::Identity, ECC_Visibility, Sphere, Params))
    {
        // Path to vertical escape is clear, now check if we can reach the desired XY from there
        FVector HighStart = Start + FVector(0, 0, VerticalEscapeHeight);
        if (!GetWorld()->SweepSingleByChannel(VerticalHit, HighStart, VerticalDesired, FQuat::Identity, ECC_Visibility, Sphere, Params))
        {
             return VerticalDesired;
        }
    }

    // If vertical escape also fails or is blocked, return the safe hit location with a small nudge
    return Hit.Location + Hit.ImpactNormal * 10.0f;
}

FVector UFloatingCompanionComponent::ComputeTargetLocation() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ZeroVector;

    if (bIsLightEnabled)
    {
        // When light is ON, stay in front of the owner
        const FVector ForwardOffset = Owner->GetActorForwardVector() * LightOnForwardDistance;
        
        // Vertical bob
        const float Bob = FMath::Sin(TimeAccumulator * ActiveConfig.BobFrequency * PI * 2.f) * ActiveConfig.BobAmplitude;

        FVector CrouchOffset = FVector::ZeroVector;
        if (const ACharacter* CharacterOwner = Cast<ACharacter>(Owner))
        {
            if (CharacterOwner->bIsCrouched)
            {
                CrouchOffset.Z = CrouchHeightOffset;
            }
        }
        
        return Owner->GetActorLocation() + ForwardOffset + BaseOffset + CrouchOffset + FVector(0.f, 0.f, Bob);
    }

    const float TotalAngle = FMath::DegreesToRadians(
        CurrentOrbitAngle + ActiveConfig.OrbitAngleOffset);

    // Orbit on XY plane
    const FVector OrbitOffset(
        FMath::Cos(TotalAngle) * ActiveConfig.OrbitRadius,
        FMath::Sin(TotalAngle) * ActiveConfig.OrbitRadius,
        0.f);

    // Vertical bob
    const float Bob = FMath::Sin(TimeAccumulator * ActiveConfig.BobFrequency * PI * 2.f)
                * ActiveConfig.BobAmplitude;

    // Crouch offset
    FVector CrouchOffset = FVector::ZeroVector;
    if (const ACharacter* CharacterOwner = Cast<ACharacter>(Owner))
    {
        if (CharacterOwner->bIsCrouched)
        {
            CrouchOffset.Z = CrouchHeightOffset;
        }
    }

    return Owner->GetActorLocation()
         + BaseOffset
         + OrbitOffset
         + CrouchOffset
         + FVector(0.f, 0.f, Bob);
}

// ─────────────────────────────────────────────
//  Morph target: blend to previous and current morphs
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::UpdateMorphTarget(float DeltaTime)
{
    if (!CompanionMesh) return;

    
    if (PreviousMorphName != NAME_None && PreviousMorphName != CurrentMorphName)
    {
        PreviousMorphValue = FMath::FInterpTo(
            PreviousMorphValue, 0.f, DeltaTime, ActiveConfig.MorphBlendSpeed);
        CompanionMesh->SetMorphTarget(PreviousMorphName, PreviousMorphValue);

        if (FMath::IsNearlyZero(PreviousMorphValue, 0.01f))
        {
            CompanionMesh->SetMorphTarget(PreviousMorphName, 0.f);
            PreviousMorphName = NAME_None;
        }
    }


    if (CurrentMorphName != NAME_None)
    {
        CurrentMorphValue = FMath::FInterpTo(
            CurrentMorphValue, ActiveConfig.MorphTargetValue,
            DeltaTime, ActiveConfig.MorphBlendSpeed);
        CompanionMesh->SetMorphTarget(CurrentMorphName, CurrentMorphValue);
    }
}

// ─────────────────────────────────────────────
//  Visuals: color and scale
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::UpdateVisuals(float DeltaTime)
{
    // color interp
    CurrentColor = FMath::CInterpTo(
        CurrentColor, ActiveConfig.Color, DeltaTime, 3.f);

    // mesh scale interp
    CurrentMeshScale = FMath::VInterpTo(
        CurrentMeshScale, ActiveConfig.MeshScale, DeltaTime, 5.f);

    if (CompanionMesh)
    {
        CompanionMesh->SetRelativeScale3D(CurrentMeshScale);
    }

    if (DynMaterial)
    {
        DynMaterial->SetVectorParameterValue(EmissiveParamName, CurrentColor);
    }
    
    // Update the point light color to match the current state color
    if (CompanionLight)
    {
        if (bSyncLightColorWithState)
        {
            CompanionLight->SetLightColor(CurrentColor);
        }
    }
}

// ─────────────────────────────────────────────
//  Polling: check environment conditions and update state
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::PollEnvironment(const float DeltaTime)
{
    PollingTimer += DeltaTime;
    if (PollingTimer < PollingInterval) return;
    PollingTimer = 0.f;
    
    // EnemyNearby
    if (IsEnemyNearby())
    {
        if (CurrentState != ECompanionState::Scared)
        {
            SetCompanionState(ECompanionState::Scared);
        }
        return;
    }

    // Go back to Idle if we were scared and no enemies are nearby
    if (CurrentState == ECompanionState::Scared)
    {
        SetCompanionState(ECompanionState::Idle);
    }
}

bool UFloatingCompanionComponent::IsEnemyNearby() const
{
    const AActor* Owner = GetOwner();
    if (!Owner || !GetWorld()) 
        return false;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    
    if (CompanionMesh) 
        Params.AddIgnoredActor(CompanionMesh->GetOwner());

    TArray<FOverlapResult> Overlaps;
    const FCollisionShape Sphere = FCollisionShape::MakeSphere(EnemyDetectionRadius);

    FCollisionObjectQueryParams ObjectParams;
    if (EnemyCollisionChannels.IsEmpty())
    {
        // Default to Pawn if no channels are specified
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    }
    else
    {
        for (const ECollisionChannel Channel : EnemyCollisionChannels)
        {
            ObjectParams.AddObjectTypesToQuery(Channel);
        }
    }

    const bool bHit = GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        Owner->GetActorLocation(),
        FQuat::Identity,
        ObjectParams,
        Sphere,
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Result : Overlaps)
        {
            if (AActor* Actor = Result.GetActor())
            {
                if (const AStillHearAICharacterBase* AIEnemy = Cast<AStillHearAICharacterBase>(Actor))
                {
                    if (const AStillHearAIControllerBase* AIController = AIEnemy->GetAICRef())
                    {
                        // If the enemy is alerted or hunting, the companion gets scared
                        if (AIController->CheckCurrentStatusTag(E_AITag::ALERTED) || AIController->CheckCurrentStatusTag(E_AITag::HUNTING))
                        {
                            return true;
                        }
                    }
                }
                else
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}


// ─────────────────────────────────────────────
//  Public API 
// ─────────────────────────────────────────────
void UFloatingCompanionComponent::SetCompanionState(ECompanionState NewState)
{
    if (NewState == CurrentState) return;

    CurrentState = NewState;
    
    // Save the previous morph for the fade-out
    PreviousMorphName  = CurrentMorphName;
    PreviousMorphValue = CurrentMorphValue;

    ActiveConfig = GetConfigForState(NewState);
    CurrentMorphName  = ActiveConfig.MorphTargetName;
    CurrentMorphValue = 0.f;  // Start from 0 and interp to MorphTargetValue
    
}

void UFloatingCompanionComponent::SetCustomState(const FCompanionStateConfig& Config)
{
    CurrentState = ECompanionState::Custom;

    PreviousMorphName  = CurrentMorphName;
    PreviousMorphValue = CurrentMorphValue;

    ActiveConfig       = Config;
    CurrentMorphName   = Config.MorphTargetName;
    CurrentMorphValue  = 0.f;

    StateConfigs.Add(ECompanionState::Custom, Config);
    
}

void UFloatingCompanionComponent::SetAutoPollingEnabled(bool bEnabled)
{
    bAutoPollingEnabled = bEnabled;
    PollingTimer = 0.f;
}

void UFloatingCompanionComponent::SetLightEnabled(const bool bEnabled, FLinearColor Color, bool bUpdateColor, float Intensity)
{
    bIsLightEnabled = bEnabled;
    
    if (CompanionLight)
    {
        CompanionLight->SetVisibility(bIsLightEnabled);
        
        if (bIsLightEnabled)
        {
            if (bUpdateColor)
            {
                CompanionLight->SetLightColor(Color);
                bSyncLightColorWithState = false;
            }

            if (Intensity >= 0.0f)
            {
                CompanionLight->SetIntensity(Intensity);
            }
        }
    }
}

void UFloatingCompanionComponent::ResetCompanionLocation()
{
    if (const AActor* Owner = GetOwner())
    {
        CurrentCompanionLocation = Owner->GetActorLocation();
        SmoothedAvoidanceTarget = CurrentCompanionLocation;
        IdealTargetLocation = CurrentCompanionLocation;
        LastProgressLocation = CurrentCompanionLocation;
        bIsAvoiding = false;
        StuckTimer = 0.f;

        if (CompanionMesh)
        {
            CompanionMesh->SetWorldLocation(CurrentCompanionLocation, false);
        }
    }
}

void UFloatingCompanionComponent::EngageInteractionSpot(UCompanionInteractionSpotComponent* Spot)
{
    if (!Spot) 
        return;
    
    ClearInteractionSpot();
    
    CurrentInteractionSpot = Spot;
    InteractionTimer = 0.f;
    InteractionOrbitAngle = 0.f;
    CachedInteractionSpline = CurrentInteractionSpot->GetAssociatedSpline();

    // Spawn VFX if configured
    if (CurrentInteractionSpot->InteractionVFX && CompanionMesh)
    {
        ActiveInteractionVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            CurrentInteractionSpot->InteractionVFX,
            CompanionMesh,
            CurrentInteractionSpot->CompanionVFXSocket,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

void UFloatingCompanionComponent::DisengageInteractionSpot()
{
    bool bIsLooping = false;
    
    if (CurrentInteractionSpot && CurrentInteractionSpot->BehaviorType == ECompanionBehavior::FollowSpline)
    {
        if (CurrentInteractionSpot->bPingPongSpline)
        {
            bIsLooping = true;
        }
        else if (USplineComponent* Spline = CurrentInteractionSpot->GetAssociatedSpline())
        {
            if (Spline->IsClosedLoop())
            {
                bIsLooping = true;
            }
        }
    }

    if (CurrentInteractionSpot && CurrentInteractionSpot->BehaviorType == ECompanionBehavior::FollowSpline && !bIsLooping)
    {
        bIsFinishingSpline = true;
    }
    else
    {
        ClearInteractionSpot();
    }
}

void UFloatingCompanionComponent::ClearInteractionSpot()
{
    CurrentInteractionSpot = nullptr;
    CachedInteractionSpline = nullptr;
    bIsFinishingSpline = false;
    
    if (ActiveInteractionVFX)
    {
        ActiveInteractionVFX->Deactivate();
        ActiveInteractionVFX = nullptr;
    }
}

const FCompanionStateConfig& UFloatingCompanionComponent::GetConfigForState(const ECompanionState State) const
{
    if (const FCompanionStateConfig* Cfg = StateConfigs.Find(State))
        return *Cfg;

    // Fallback to config Idle if the state not configured
    if (const FCompanionStateConfig* Idle = StateConfigs.Find(ECompanionState::Idle))
        return *Idle;
    
    // Absolute fallback (should never reach here)
    static FCompanionStateConfig DefaultCfg;
    return DefaultCfg;
}
