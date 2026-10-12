#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "StillHearCompanionAIController.generated.h"

UCLASS()
class STILLHEAR_API AStillHearCompanionAIController : public AAIController
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Event")
	FGameplayTag EventTagToSendWhenRecalled;
	UPROPERTY(VisibleAnywhere, Category = "Current Target Actor")
	TObjectPtr<class AActor> TargetActor;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	AStillHearCompanionAIController();
#pragma endregion 

#pragma region METHODS
public:
	// Companion Input Handling
	void HandleCompanionMovementInput(const FVector& Right, const FVector& Forward, FVector2D InputValue) const;
	void HandleStartCompanionSoundWaveInput();
	void HandleShootCompanionSoundWaveInput();
	void HandleInterruptCompanionSoundWaveInput();
	void HandleCompanionSwitchTargetInput(const FVector2D InputDir2D) const;
	
	void MoveToTargetActor(AActor* TargetActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult & Result) override;

private:
	void SendEventToTargetActor() const;
#pragma endregion 
};
