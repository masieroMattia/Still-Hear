// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"
#include "EnemiesAI/Utility/DataAssets/AIWormInfo_DataAsset.h"
#include "AIWormController.generated.h"

UCLASS()
class STILLHEAR_API AAIWormController : public AStillHearAIControllerBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
	UPROPERTY()
	TObjectPtr<AAIWormCharacter> NPCWormRef;

	UPROPERTY()
	TObjectPtr<class UWormAnimInstance> WormAnimInstance;

	UPROPERTY()
	TObjectPtr<UAIWormInfo_DataAsset> NPCWormDataAsset;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;
#pragma endregion
	
#pragma region VARIABLES
protected:
	// Reference to return to patrol after a certain time the worm doesn't hear sound
	FTimerHandle AlertCooldownTimerHandle;

	// Reference to the Target Location, used by Abilities, updated together with blackboard key
	FVector CurrentTargetLocation;

	// Diving Status
	bool bIsInAir;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	AAIWormController();
#pragma endregion

#pragma region UFUNCTION
protected:
	UFUNCTION()
	void OnStatusTagChangedCallback(FGameplayTag NewStatusTag);
#pragma endregion 
	
#pragma region METHODS
public:
	virtual void Tick(float DeltaTime) override;

	class AAIWormCharacter* GetWormRef() const { return Cast<AAIWormCharacter>(NPCRef); }
	class UAIWormInfo_DataAsset* GetDataAsset() const { return Cast<UAIWormInfo_DataAsset>(NPCRef->GetAIInfo_DataAsset()); }

	FVector GetCurrentTargetLocation() const { return CurrentTargetLocation; }
	void SetCurrentTargetLocation(const FVector& NewTargetLocation) { CurrentTargetLocation = NewTargetLocation; }

	AActor* GetCurrentTargetActor() const { return CurrentTargetActor; }

	void SetIsDiving(const bool bIsDiving);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupHearingInfo() override;

	virtual void PerceptionEventReceived(AActor* UpdatedActor, FAIStimulus Stimulus) override;
private:
	void UpdateTargetLocation(const FVector& TargetLocation, bool WasBell = false);
	void UpdateTargetActor(AActor* TargetActor);
	void ClearAlertStatus();

	void HandleAlertTimer(bool WasBell = false);

	void UpdateAnimInstance();
#pragma endregion 
};
