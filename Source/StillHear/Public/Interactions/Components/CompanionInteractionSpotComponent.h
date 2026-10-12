#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CompanionInteractionSpotComponent.generated.h"

class UNiagaraSystem;
class USplineComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class ECompanionBehavior : uint8
{
    AttachToSpot  UMETA(DisplayName = "Attach To Spot"),
    OrbitAround   UMETA(DisplayName = "Orbit Around Spot"),
    FollowSpline  UMETA(DisplayName = "Follow Spline")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UCompanionInteractionSpotComponent : public USceneComponent
{
    GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
    UCompanionInteractionSpotComponent();
#pragma endregion

#pragma region UPROPERTIES
public:

    // The behavior the companion will adopt when interacting with this spot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings")
    ECompanionBehavior BehaviorType = ECompanionBehavior::AttachToSpot;

    // Visual effect to spawn when the companion engages this spot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|VFX")
    TObjectPtr<UNiagaraSystem> InteractionVFX;

    // Socket or bone name on the companion mesh to attach the VFX to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|VFX")
    FName CompanionVFXSocket = NAME_None;

    // How fast the companion interpolates towards the target location during this interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings")
    float InteractionFollowSpeed = 15.0f;

    // ── Orbit Settings ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|Orbit", meta=(EditCondition="BehaviorType == ECompanionBehavior::OrbitAround", EditConditionHides))
    float OrbitRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|Orbit", meta=(EditCondition="BehaviorType == ECompanionBehavior::OrbitAround", EditConditionHides))
    float OrbitSpeed = 90.0f;

    // The exact name of the Spline Component on this actor to follow
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|Spline", meta=(EditCondition="BehaviorType == ECompanionBehavior::FollowSpline", EditConditionHides))
    FName SplineNameTag;

    // If true, the companion will continuously travel back and forth along the spline until interaction stops
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|Spline", meta=(EditCondition="BehaviorType == ECompanionBehavior::FollowSpline", EditConditionHides))
    bool bPingPongSpline = false;

    // Curve dictating the progress along the spline. X is time (Seconds), Y is distance (0 to 1). The max time of the curve determines the Spline Duration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion Settings|Spline", meta=(EditCondition="BehaviorType == ECompanionBehavior::FollowSpline", EditConditionHides))
    TObjectPtr<UCurveFloat> SplineProgressCurve;
#pragma endregion

#pragma region METHODS
public:
    // Finds the associated spline component on the owner
    USplineComponent* GetAssociatedSpline() const;
#pragma endregion
};
