#pragma once

#include "CoreMinimal.h"
#include "InteractableObj.h"
#include "Components/TimelineComponent.h"
#include "Interactions/Utils/InteractableRail.h"
#include "RailMoveObj.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UNiagaraSystem;
class UCurveFloat;
class USoundBase;

UENUM(BlueprintType)
enum class ERailActivationType : uint8
{
	Both		 UMETA(DisplayName = "Both (Player and Trigger)"),
	PlayerOnly   UMETA(DisplayName = "Player Only"),
	TriggerOnly  UMETA(DisplayName = "Trigger Only"),
	None		 UMETA(DisplayName = "None (Sound Only)")
};

UCLASS()
class STILLHEAR_API ARailMoveObj : public AInteractableObj
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Rail")
	TObjectPtr<AInteractableRail> RailActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	ERailActivationType ActivationType = ERailActivationType::Both;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	TObjectPtr<UCurveFloat> MovementCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	float HeightOffset = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	FRotator RotationOffset = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bFollowSlope = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> StartMoveSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> MoveSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> ArriveSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> StartMoveEffect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> MoveEffect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> ArriveEffect;
#pragma endregion
	
#pragma region VARIABLES
private:
	float CurrentDistance = 0.0f;
	float SplineLength = 0.0f;
	bool bIsMoving = false;
	float MoveDirection = 1.0f; // 1.0f for forward, -1.0f for backward

	FTimeline MovementTimeline;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> SpawnedAudio;
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedNiagara;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	ARailMoveObj();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateLocationOnRail(float Distance);

	UFUNCTION()
	void HandleMovementProgress(float Value);
	UFUNCTION()
	void HandleMovementFinished();
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	void SaveCheckpointStateAtRailEnd();

protected:
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void TriggerInteraction(AActor* Triggerer = nullptr) override;
#pragma endregion
};