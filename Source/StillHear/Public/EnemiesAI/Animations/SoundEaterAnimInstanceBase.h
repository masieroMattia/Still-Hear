#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SoundEaterAnimInstanceBase.generated.h"

class AAIMantisCharacter;

UCLASS(Abstract, NotBlueprintable)
class STILLHEAR_API USoundEaterAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly)
	FVector CurrentVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	float CurrentDirection = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float Speed = 200.0f;
	UPROPERTY(BlueprintReadOnly)
	bool bIsStunned = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsFalling = false;
	UPROPERTY(BlueprintReadOnly)
	bool bShouldMove = false;
#pragma endregion
	
#pragma region VARIABLES
	UPROPERTY()
	TObjectPtr<AAIMantisCharacter> Pawn = nullptr;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
#pragma endregion
};
