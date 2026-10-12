// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WormAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UWormAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	TObjectPtr<class AAIWormCharacter> Worm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	FVector LookAtPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsRoaring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsDiving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsAlerted;
#pragma endregion 
	
#pragma region METHODS
public:
	void SetIsWalking(const bool Value) { bIsWalking = Value; }
	void SetIsRunning(const bool Value) { bIsRunning = Value; }
	void SetIsRoaring(const bool Value) { bIsRoaring = Value; }
	void SetIsDiving(const bool Value) { bIsDiving = Value; }
	void SetIsAlerted(const bool Value) { bIsAlerted = Value; }
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
#pragma endregion 
};
