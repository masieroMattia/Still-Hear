#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_NavigateTo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNavigateToDelegate);

UCLASS()
class STILLHEAR_API UAT_NavigateTo : public UAbilityTask
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY()
	FNavigateToDelegate OnTargetLocationReached;
#pragma endregion
	
#pragma region VARIABLES
protected:
	FVector TargetLocation;
	FVector OriginalTargetLocation; // The original target, used for rotation after reaching the navigation point
	FVector FacingDirection;
	
	float AcceptanceRadius;
	bool bHasReachedTarget = false;
	bool bHasStartedMoving = false; // True once the character has started moving, prevents premature stop detection
	float TimeSinceActivation = 0.0f; // Tracks time since task activation, used as a grace period for movement start
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UAT_NavigateTo();
#pragma endregion
	
#pragma region UFUNCTIONS
	UFUNCTION()
	static UAT_NavigateTo* NavigateTo(UGameplayAbility* OwningAbility, const FVector& InTargetLocation, float InAcceptanceRadius, const FVector& InFacingDirection = FVector::ZeroVector);
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
#pragma endregion
};
