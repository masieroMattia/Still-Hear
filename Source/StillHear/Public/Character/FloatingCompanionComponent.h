#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FloatingCompanionComponent.generated.h"

class UCompanionInteractionSpotComponent;
class UPointLightComponent;

// ─────────────────────────────────────────────
//  Enum for states
// ─────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECompanionState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Scared          UMETA(DisplayName = "Scared"),
    Happy           UMETA(DisplayName = "Happy"),
    Angry           UMETA(DisplayName = "Angry"),
    Interactable    UMETA(DisplayName = "Interactable Nearby"),
    Custom          UMETA(DisplayName = "Custom"),
};

// ─────────────────────────────────────────────
//  Configuration struct for states
// ─────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCompanionStateConfig
{
    GENERATED_BODY()
    
    // Morph target name to active on this state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    FName MorphTargetName = NAME_None;

    // Target value for the morph target (0..1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MorphTargetValue = 1.0f;
    
    // Interpolation speed to this state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph")
    float MorphBlendSpeed = 3.0f;

    // Bob offset (vertical oscillation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
    float BobAmplitude = 15.0f;
    
    // Oscillation frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
    float BobFrequency = 1.5f;
    
    // Orbital offset around the character (angle in degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
    float OrbitAngleOffset = 0.0f;
    
    // Orbital distance from the character 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
    float OrbitRadius = 80.0f;
    
    // Orbiting speed (degrees/sec, 0 = no orbit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
    float OrbitSpeed = 0.0f;
    
    // Color of the mesh for this state (used via Dynamic Material)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor Color = FLinearColor::White;

    // Mesh scale for this state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FVector MeshScale = FVector::OneVector;
};

// ─────────────────────────────────────────────
//  Component
// ─────────────────────────────────────────────
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STILLHEAR_API UFloatingCompanionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFloatingCompanionComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── public API ──────────────────────────────────────────
    
    /** Change state of companion. Starts morph/visual transition. */
    UFUNCTION(BlueprintCallable, Category = "Companion")
    void SetCompanionState(ECompanionState NewState);

    /** Current status */
    UFUNCTION(BlueprintPure, Category = "Companion")
    ECompanionState GetCurrentState() const { return CurrentState; }
    
    /** Current position */
    UFUNCTION(BlueprintPure, Category = "Companion")
    FVector GetCurrentLocation() const { return CurrentCompanionLocation; }
    
    /** Force the companion into a custom state with runtime configuration */
    UFUNCTION(BlueprintCallable, Category = "Companion")
    void SetCustomState(const FCompanionStateConfig& Config);
    
    /** Enable/disable automatic polling for enemies and HP */ 
    UFUNCTION(BlueprintCallable, Category = "Companion|Polling")
    void SetAutoPollingEnabled(bool bEnabled);

    /** Enable/disable the point light (if configured) */
    UFUNCTION(BlueprintCallable, Category = "Companion|Light")
    void SetLightEnabled(bool bEnabled, FLinearColor Color = FLinearColor::White, bool bUpdateColor = false, float Intensity = -1.0f);
    
    /** Reset the companion location immediately to the owner's location */
    UFUNCTION(BlueprintCallable, Category = "Companion")
    void ResetCompanionLocation();

    // ── Interaction API ──────────────────────────────────────
    
    /** Start an interaction behavior using a Spot Component */
    UFUNCTION(BlueprintCallable, Category = "Companion|Interaction")
    void EngageInteractionSpot(UCompanionInteractionSpotComponent* Spot);

    /** End the current interaction behavior */
    UFUNCTION(BlueprintCallable, Category = "Companion|Interaction")
    void DisengageInteractionSpot();

    /** Completely clear the interaction spot */
    void ClearInteractionSpot();
    
    // ── Configuration ────────────────────────────────────────
    
    /** Skeletal mesh of the companion (automatically created if null) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Companion|Components")
    TObjectPtr<USkeletalMeshComponent> CompanionMesh;
    
    /** Skeletal mesh asset to use for the companion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Setup")
    TObjectPtr<USkeletalMesh> CompanionMeshAsset;
    
    /** Point light attached to the companion */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Companion|Components")
    TObjectPtr<UPointLightComponent> CompanionLight;
    
    /** Configuration for each state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|States")
    TMap<ECompanionState, FCompanionStateConfig> StateConfigs;
    
    /** Base offset from the character (before bob/orbit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Float")
    FVector BaseOffset = FVector(0.f, 0.f, 120.f);
    
    /** Speed at which the companion follows the target position (smoothing) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Float")
    float FollowSpeed = 8.0f;

    /** Vertical offset applied when the owner is crouching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Float")
    float CrouchHeightOffset = -60.0f;

    /** If true, the companion will perform traces to avoid passing through world geometry */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    bool bPreventClipping = true;

    /** Radius of the sphere trace used for clipping avoidance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float CollisionSphereRadius = 20.0f;

    /** Interval for the custom tick timer used for obstacle avoidance checks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float ObstacleCheckInterval = 0.1f;

    /** How far the companion must be from its target before the stuck-timer starts (squared internally) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float StuckDistanceThreshold = 50.0f;

    /** Seconds the companion can be stuck before it teleports to the player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float StuckTeleportTime = 2.5f;

    /** Maximum distance from owner before a catch-up boost kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float MaxLagDistance = 800.0f;

    /** Height to try when deflection along the surface fails */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float VerticalEscapeHeight = 80.0f;

    /** Speed at which the companion smoothly transitions toward avoidance positions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Collision")
    float AvoidanceSmoothSpeed = 6.0f;
    
    /** Radius to search for enemies (auto-polling) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Polling")
    float EnemyDetectionRadius = 600.f;
    
    /** Interval between automatic polling checks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Polling")
    float PollingInterval = 0.5f;
    
    /** Collision channels to search for enemies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Polling")
    TArray<TEnumAsByte<ECollisionChannel>> EnemyCollisionChannels;
    
    /** Name of the emissive parameter in the material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Material")
    FName EmissiveParamName = TEXT("EmissiveColor");

    /** Whether the point light color should match the companion's current state color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Light")
    bool bSyncLightColorWithState = false;

    /** Whether the point light is enabled (if CompanionLight is set) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Light")
    bool bIsLightEnabled = true;

    /** Distance from the character to float in front when the light is ON */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Light")
    float LightOnForwardDistance = 200.0f;

private:
    // ── Runtime state ─────────────────────────────────────────

    ECompanionState CurrentState = ECompanionState::Idle;
    FCompanionStateConfig ActiveConfig;
    FCompanionStateConfig PreviousConfig;
    
    // Current world location of the companion, updated every tick
    FVector CurrentCompanionLocation;

    // ── Obstacle Avoidance / Stuck Detection ─────────────────
    FTimerHandle ObstacleCheckTimerHandle;
    FVector IdealTargetLocation = FVector::ZeroVector;

    float StuckTimer = 0.f;
    FVector LastProgressLocation = FVector::ZeroVector;
    FVector SmoothedAvoidanceTarget = FVector::ZeroVector;
    bool bIsAvoiding = false;

    // Accumulator for bob and orbit
    float TimeAccumulator = 0.f;
    float CurrentOrbitAngle = 0.f;
    
    // Current morph value (interpolated)
    float CurrentMorphValue = 0.f;
    FName CurrentMorphName = NAME_None;
    FName PreviousMorphName = NAME_None;
    float PreviousMorphValue = 0.f;

    // Current scale (interpolated)
    FVector CurrentMeshScale = FVector::OneVector;
    
    // Current color (interpolated)
    FLinearColor CurrentColor = FLinearColor::White;

    // Dynamic Material Instance
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynMaterial;

    // Polling
    bool bAutoPollingEnabled = true;
    float PollingTimer = 0.f;

    // ── Interaction State ─────────────────────────────────────
    
    UPROPERTY()
    TObjectPtr<UCompanionInteractionSpotComponent> CurrentInteractionSpot;
    
    UPROPERTY()
    TObjectPtr<class UNiagaraComponent> ActiveInteractionVFX;

    UPROPERTY()
    TObjectPtr<class USplineComponent> CachedInteractionSpline;

    float InteractionTimer = 0.f;
    float InteractionOrbitAngle = 0.f;
    bool bIsFinishingSpline = false;

    // ── Private Methods ────────────────────────────────────────
    
    void SetupCompanionMesh();
    void UpdateFloating(float DeltaTime);
    void UpdateMorphTarget(float DeltaTime);
    void UpdateVisuals(float DeltaTime);
    void PollEnvironment(float DeltaTime);
    FVector ComputeTargetLocation() const;
    bool IsEnemyNearby() const;
    const FCompanionStateConfig& GetConfigForState(ECompanionState State) const;
    
    /** Attempt to move from Current to Desired, trying deflection and vertical escape on hit. */
    FVector SolveObstacleAvoidance(const FVector& Current, const FVector& Desired, const AActor* IgnoreActor) const;
    void PerformObstacleCheck();
};

