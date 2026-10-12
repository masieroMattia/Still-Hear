#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interfaces/IKTargetReceiver.h"
#include "MainCharacterAnimInstance.generated.h"

UCLASS()
class STILLHEAR_API UMainCharacterAnimInstance : public UAnimInstance, public IIKTargetReceiver
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	TObjectPtr<class AStillHearMainCharacter> Character;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	TObjectPtr<class UCharacterMovementComponent> MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	FVector Velocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	float GroundSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	float Direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	bool bShouldMove;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	bool bIsFalling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	bool bIsCrouching;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
	bool bIsSprinting;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlendSpace")
	TObjectPtr<UBlendSpace> LocomotionBlendSpace;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKTrace")
	float IKTraceAlpha;
	// Speed at which the hands blend towards the interaction spots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKTrace|Settings")
	float IKBlendInSpeed = 10.0f;
	// Speed at which the hands return to their normal animation state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKTrace|Settings")
	float IKBlendOutSpeed = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKTrace|LeftHand")
	FTransform LeftEffectorTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKTrace|RightHand")
	FTransform RightEffectorTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	bool bIsClimbing;
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsDragging = false;

	// True while the character should be in the looping seated-idle pose
	UPROPERTY(BlueprintReadOnly, Category = "Sitting")
	bool bIsSitting = false;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	virtual void UpdateIKTargets(const FTransform& LeftTarget, const FTransform& RightTarget) override;
};
