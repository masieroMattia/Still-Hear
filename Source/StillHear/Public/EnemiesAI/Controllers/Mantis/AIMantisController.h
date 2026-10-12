#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Utility/DataAssets/AIMantisInfo_DataAsset.h"
#include "EnemiesAI/Controllers/Base/StillHearAIControllerBase.h"
#include "AIMantisController.generated.h"

class UPerceptionsMeterComponent;

UCLASS()
class STILLHEAR_API AAIMantisController : public AStillHearAIControllerBase
{
	GENERATED_BODY()
#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPerceptionsMeterComponent> PerceptionsMeterComp;
#pragma endregion
	
#pragma region VARIABLES
protected:
	FTimerHandle LoseSightTimerHandle;
	FTimerHandle DisturbanceCooldownTimerHandle;
	FTimerHandle ContinuousSightTimerHandle;
	
	bool HasLoseSight = false;
	bool IsDisturbanceCooldownActive = false;

	// Guard flag to prevent infinite propagation loops between nearby AIs
	bool bIsPropagating = false;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	AAIMantisController();
#pragma endregion

#pragma region UFUNCTIONS
public:
	E_AISightCone GetTypeOfSightCone(const AActor* UpdatedActor) const;
	
protected:
	virtual void PerceptionEventReceived(AActor* UpdatedActor, FAIStimulus Stimulus) override;

private:
	void StartContinuousSightUpdate();
	void StopContinuousSightUpdate();
	
	UFUNCTION()
	void UpdateSightLogic();
	UFUNCTION()
	void OnLoseSightTimerFinished();

	// Finds all nearby Mantis within GroupAwarenessRadius and calls ReceiveGroupAlert on them
	void PropagateGroupAwareness(E_AITag NewTag, AActor* TargetActor, const FVector& DisturbanceLoc) const;

	// Callback bound to OnStatusTagChanged to trigger group propagation
	UFUNCTION()
	void OnStatusTagChangedCallback(FGameplayTag NewStatusTag);
#pragma endregion 
	
#pragma region METHODS
public:
	virtual void Tick(float DeltaTime) override;

	AAIMantisCharacter* GetMantisRef() const { return Cast<AAIMantisCharacter>(NPCRef); }
	UAIMantisInfo_DataAsset* GetDataAsset() const { return Cast<UAIMantisInfo_DataAsset>(NPCRef->GetAIInfo_DataAsset()); }
	UPerceptionsMeterComponent* GetPerceptionsMeterComponent() const { return PerceptionsMeterComp; }
	
	void UpdateTarget(AActor* UpdatedActor);
	void UpdateDisturbanceLocation(const AActor* DisturbanceActor);

	// Immediately makes the Mantis target and hunt the given actor, skipping the Awareness/Alert
	void ForceHuntTarget(AActor* TargetActor);
	
	void ClearDisturbanceCooldown();
	void ClearLoseSight();
	
	void ClearTargetKey();
	void ClearTargetLocationKey();

	// Called by a nearby Mantis to propagate its state to this AI
	void ReceiveGroupAlert(E_AITag IncomingTag, AActor* TargetActor, const FVector& DisturbanceLoc);

	virtual void ResetAIState() override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupSightInfo() override;
	virtual void SetupHearingInfo() override;
	void SetupTouchInfo() const;
	bool IsStunned() const;
	
	virtual void SetupBlackboardKeys() override;
#pragma endregion 
};
