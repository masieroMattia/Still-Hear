#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MantisAnimInstance.generated.h"

class UCharacterMovementComponent;
class UAbilitySystemComponent;
class AAIMantisCharacter;

UCLASS()
class STILLHEAR_API UMantisAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
#pragma region VARIABLES
protected:
	// Current velocity of the Mantis
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;
	
	// Ground speed of the Mantis
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;
	
	// Direction of the Mantis
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;
	
	// Check if the Mantis is currently moving
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	// Check if the Mantis is in the air (falling/jumping)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsStunned = false;
	
	// Reference to the Pawn owning this AnimInstance
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<AAIMantisCharacter> MantisRef;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> MantisASC;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MantisCMC;
#pragma endregion
	
#pragma region METHODS
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
#pragma endregion 
};
