#pragma once

#include "CoreMinimal.h"
#include "TraceAndCollision/CustomCollision.h"
#include "AIPerceptionsMeterInfo_DataAsset.h"
#include "AIMantisInfo_DataAsset.generated.h"

UCLASS()
class STILLHEAR_API UAIMantisInfo_DataAsset : public UAIPerceptionsMeterInfo_DataAsset
{
	GENERATED_BODY()

public:
	// ===== MOVEMENT =====

	// Radius used by the EQS query to find random points
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float EQSRadius = 500.f;

	// Minimum distance between each EQS-generated points
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float EQSDistanceFromEach = 350.f;
	
	// ===== SIGHT =====
	// Configures the Unreal AIPerception sight sense and the custom multi-cone system.

	// Vertical height offset for the AI's vision viewpoint (GetActorEyesViewPoint)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float SightHeight = 90.0f;

	// Maximum vertical height difference (Z-axis distance) at which the AI can see the target
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float MaxSightHeightDifference = 300.0f;

	// Maximum distance at which the AI can detect targets (fed into UE AIPerception SightConfig)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float SightRadius = 1200.0f;

	// Distance at which a previously seen target is considered lost (fed into UE AIPerception SightConfig)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float LoseSightRadius = 1400.0f;

	// Seconds after losing sight before the AI reacts (transitions from ALERTED to HUNTING and clears the target)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float LoseSightTimer = 2.0f;

	// Half-angle in degrees for the UE AIPerception peripheral vision
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Sight")
	float PeripheralVisionAngleDegrees = 110.0f;

	// Maximum distance for the Narrow cone detection
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightRadius_Narrow = 600.0f;

	// Half-angle in degrees for the Narrow cone (front center)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightPeripheralHalfAngleDegree_Narrow = 25.0f;

	// Maximum distance for the Wide cone detection
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightRadius_Wide = 800.0f;

	// Half-angle in degrees for the Wide cone (moderately off-center)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightPeripheralHalfAngleDegree_Wide = 60.0f;

	// Maximum distance for the Peripheral cone detection
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightRadius_Peripheral = 400.0f;

	// Half-angle in degrees for the Peripheral cone (far to the sides)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightPeripheralHalfAngleDegree_Peripheral = 90.0f;

	// Maximum distance at which the AI can sense a target directly behind it
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Perception | Sight")
	float SightRadius_Backward = 200.0f;

	// ===== HEARING =====

	// Hearing detection range when the player is walking.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Hearing")
	float WalkHearingRange = 350.0f;

	// Max Hearing detection range
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Hearing")
	float RunHearingRange = 900.0f;
	
	// ===== TOUCH =====
	// Canal used to report the touch to the AIPerception component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception | Touch")
	TEnumAsByte<ECollisionChannel> TouchReportChannel = ECustomCollision::Player;

	// ===== BEHAVIOR TREE =====

	// Minimum time in seconds between two consecutive DisturbanceLocation updates
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BT | Disturbance")
	float DisturbanceCooldownTimer = 1.5f;

	// Delay in seconds before the AI starts moving towards the disturbance when entering the Investigating state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BT | Investigate")
	float StartInvestigatingWaitTime = 0.8f;

	// How long in seconds the AI investigates the disturbance area before giving up and returning to patrol
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BT | Investigate")
	float InvestigateTimer = 20.0f;

	// Delay in seconds before the AI starts actively hunting after entering the Hunting state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BT | Hunting")
	float StartHuntingWaitTime = 0.3f;

	// How long in seconds the AI hunts for the player before giving up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BT | Hunting")
	float HuntingTimer = 25.0f;

	// ===== GROUP AWARENESS =====
	// When enabled, a Mantis that changes state will propagate its awareness to nearby allies

	// If true, when this AI becomes SUSPICIOUS, nearby Mantis within GroupAwarenessRadius also become SUSPICIOUS
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Group Awareness")
	bool bEnableSuspiciousPropagation = false;

	// If true, when this AI becomes ALERTED, nearby Mantis within GroupAwarenessRadius also become ALERTED and receive the TargetActor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Group Awareness")
	bool bEnableAlertedPropagation = false;

	// Maximum distance (in cm) within which nearby Mantis will receive the propagated state
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Group Awareness", meta = (EditCondition = "bEnableSuspiciousPropagation || bEnableAlertedPropagation"))
	float GroupAwarenessRadius = 2000.0f;
};
