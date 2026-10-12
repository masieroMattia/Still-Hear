#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "EnemiesAI/Utility/DataAssets/AIMantisInfo_DataAsset.h"
#include "AIMantisCharacter.generated.h"

class UAnimMontage;
class UBoxComponent;
class UWidgetComponent;
class UPerceptionVisualizerComponent;

UCLASS()
class STILLHEAR_API AAIMantisCharacter : public AStillHearAICharacterBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UPerceptionVisualizerComponent> PerceptionVisualizerComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat")
	TObjectPtr<UBoxComponent> AttackHitBox;

	// If true, the Mantis spawns completely idle
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Dormancy")
	bool bStartDormant = false;

	// Montage to play when the Mantis is activated from a dormant state
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Dormancy", meta = (EditCondition = "bStartDormant", EditConditionHides))
	TObjectPtr<UAnimMontage> WakeUpMontage;

	// Distance to search for a valid NavMesh point when the Mantis is outside the mesh
	UPROPERTY(EditDefaultsOnly, Category = "AI|Navigation")
	float NavMeshSearchRadius = 500.0f;

	// Interval (seconds) for the periodic NavMesh consistency check
	UPROPERTY(EditDefaultsOnly, Category = "AI|Navigation")
	float NavMeshCheckInterval = 5.0f;

	// Number of consecutive failed checks required before snapping back to the NavMesh (avoids correcting on isolated false positives)
	UPROPERTY(EditDefaultsOnly, Category = "AI|Navigation")
	int32 NavMeshCheckFailureThreshold = 3;
#pragma endregion

#pragma region VARIABLES
protected:
	bool bIsDormant = false;

	// True while the Mantis is executing a NavLink ballistic jump
	bool bIsPerformingNavLinkJump = false;

	FTimerHandle NavMeshCheckTimerHandle;

	// Counts consecutive failed NavMesh consistency checks
	int32 ConsecutiveNavMeshCheckFailures = 0;
#pragma endregion
	
#pragma region CONSTRUCTOR
	AAIMantisCharacter();
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintPure, Category = "Data Assets")
	UAIMantisInfo_DataAsset* GetMantisDataAsset() const { return Cast<UAIMantisInfo_DataAsset>(GetAIInfo_DataAsset()); }

	// Wakes the Mantis up
	UFUNCTION(BlueprintCallable, Category = "Dormancy")
	void Activate();

	UFUNCTION(BlueprintPure, Category = "Dormancy")
	bool IsDormant() const { return bIsDormant; }

	// NavLink jump state
	bool IsPerformingNavLinkJump() const { return bIsPerformingNavLinkJump; }
	void SetIsPerformingNavLinkJump(bool bJumping);
#pragma endregion
	
#pragma region METHODS
public:
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void NotifyHitByProjectile(AActor* HitInstigator) override;

	void EnterDormancy();

protected:
	virtual void ResetAfterDeath() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

private:
	// Called periodically to ensure the Mantis hasn't fallen off the NavMesh
	void CheckNavMeshConsistency();

	// Callback when the wake-up montage completes or is interrupted
	void OnWakeUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// Restarts the Behavior Tree, perception, and enables walking mode
	void FinishActivation();
#pragma endregion
};