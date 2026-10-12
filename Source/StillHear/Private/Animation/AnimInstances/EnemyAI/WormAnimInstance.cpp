// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstances/EnemyAI/WormAnimInstance.h"

#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"

void UWormAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Worm = Cast<AAIWormCharacter>(GetOwningActor());
}

void UWormAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(Worm))
	{
		LookAtPos = Worm->GetLookAtPosLocation();
	}
}
