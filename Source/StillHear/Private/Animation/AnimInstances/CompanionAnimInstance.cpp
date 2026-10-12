// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstances/CompanionAnimInstance.h"

#include "Character/StillHearCompanionCharacter.h"

void UCompanionAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CompanionCharacter = Cast<AStillHearCompanionCharacter>(GetOwningActor());
}

void UCompanionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CompanionCharacter)
		return;

	CurrentStatus = CompanionCharacter->GetStatus();
}
