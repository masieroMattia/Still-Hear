#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Utility/AIEnum.h"
#include "Character/StillHearCharacterBase.h"
#include "StillHearAICharacterBase.generated.h"

class AStillHearAIControllerBase;
class UAIInfo_DataAssetBase;
class UIndicatorDescriptor;
class UIndicatorWidgetBase;
class UIndicatorComponent;
class ATargetPoint;
class AWaypoint;

UCLASS()
class STILLHEAR_API AStillHearAICharacterBase : public AStillHearCharacterBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UIndicatorComponent> IndicatorComponent;
	
	UPROPERTY()
	TObjectPtr<AStillHearAIControllerBase> AICRef;

	UPROPERTY(EditDefaultsOnly, Category = "AI Type")
	E_AIType AIType;

	UPROPERTY(EditAnywhere, Category = "Data Assets")
	TObjectPtr<UAIInfo_DataAssetBase> AIInfo_DataAsset;

	// For patrolling
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Waypoint")
	TObjectPtr<AWaypoint> StartingWaypoint;

	UPROPERTY(BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<AWaypoint> CurrentWaypoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USceneComponent> AttackFeedbackVfxLocation;
#pragma endregion

#pragma region VARIABLES
protected:
	E_AISpeedType CurrentSpeedType;

	FVector OriginalLocation;
	FRotator OriginalRotation;
	
	// Saved air control value to restore after jumps
	float DefaultAirControl = 0.05f;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	AStillHearAICharacterBase();
#pragma endregion 

#pragma region UFUNCTION()
public:
	// Reference to the controller already with the correct type to avoid multiple useless casts
	UFUNCTION(BlueprintPure, Category = "AIC")
	AStillHearAIControllerBase* GetAICRef() const { return AICRef; }

	UFUNCTION(BlueprintPure, Category = "AI Type")
	E_AIType GetAIType() const { return AIType; }

	UFUNCTION(BlueprintCallable, Category = "Speed")
	void ChangeSpeedType(E_AISpeedType NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Speed")
	E_AISpeedType GetSpeedType() const { return CurrentSpeedType; }

	UFUNCTION(BlueprintPure, Category = "Data Assets")
	UAIInfo_DataAssetBase* GetAIInfo_DataAsset() const { return AIInfo_DataAsset; }

	UFUNCTION(BlueprintCallable, Category = "Collision")
	virtual void SetCollision(const TArray<TEnumAsByte<ECollisionChannel>>& Channels, bool bIgnore);

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	AWaypoint* GetStartingWaypoint() const { return StartingWaypoint; }

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	AWaypoint* GetCurrentWaypoint() const { return CurrentWaypoint; }

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void SetCurrentWaypoint(AWaypoint* NewWaypoint) { CurrentWaypoint = NewWaypoint; }

#pragma endregion

#pragma region METHODS	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void ResetAfterDeath();
	virtual void HandleStun(const FGameplayTag Tag, int32 NewCount);
	virtual void HandleAttack(const FGameplayTag Tag, int32 NewCount);

public:
	// Reaction hook called when struck by a player-fired projectile
	virtual void NotifyHitByProjectile(AActor* HitInstigator) {}
#pragma endregion
};