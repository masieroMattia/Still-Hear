// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/CompanionStatus.h"
#include "CompanionAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UCompanionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	TObjectPtr<class AStillHearCompanionCharacter> CompanionCharacter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateTracking")
	ECompanionStatus CurrentStatus;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
