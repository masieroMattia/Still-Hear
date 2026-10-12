#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interactions/Actors/InteractableObj.h"
#include "PushObj.generated.h"

class UGameplayEffect;
class IIKTargetReceiver;
class UPushSpotComponent;
class UPushInteractionData;
class UCharacterMovementComponent;

UCLASS()
class STILLHEAR_API APushObj : public AInteractableObj
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// Box collision used as the new Root Component to handle world collisions properly
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBoxComponent> PushBoxCollision;
	// Empty scene component used as a folder to group all the push spots in the editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PushSpotsRoot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Effect")
	TSubclassOf<UGameplayEffect> DragEffectClass;
	
	// Multiplier applied to the character's speed when dragging the object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = "Interactable Settings|Drag")
	float DragMultiplier = 0.5f;
	
	// Height offset applied to the mesh when dragging
	UPROPERTY(EditAnywhere, Category = "Interactable Settings|Lift")
	float LiftingZOffset = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Interactable Settings|Lift")
	float LiftingLerpSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> DragInteractionSound;
	
	// Actions to display in the prompt while the object is being pushed/dragged
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Input")
	TArray<TObjectPtr<UInputAction>> PushingPromptActions;
	// Custom separator to use while pushing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Input")
	TSubclassOf<UUserWidget> PushingPromptSeparatorClass;
#pragma endregion
	
#pragma region VARIABLES
	float CurrentLiftingOffset = 0.0f;
	float TargetLiftingOffset = 0.0f;
	float InteractorToSpotsZ = 0.0f;
	float InitialPushSpotsRootRelativeZ = 0.0f;
	
	// The specific spot the interactor is currently using
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UPushSpotComponent> ActivePushSpot;
	UPROPERTY()
	TObjectPtr<ACharacter> CurrentInteractor;
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> CurrentInteractorMesh;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CurrentInteractorASC;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CurrentInteractorMovementComponent;
	UPROPERTY()
	TArray<TObjectPtr<UPushSpotComponent>> PushSpots;
	UPROPERTY()
	TObjectPtr<UAudioComponent> SpawnedAudio;
	
	bool bWasMovingLastFrame = false;
	
	FVector AttachmentOffset;
	
	// Stores the unique ID of the speed effect to safely remove it later
	FActiveGameplayEffectHandle CurrentSpeedEffectHandle;
#pragma endregion
	
#pragma region COSTRUCTOR
public:
	APushObj();
#pragma endregion

#pragma region METHODS
public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
protected:
	virtual void BeginPlay() override;
	
	void HandleLowering();
	void UpdateMovement();
	bool TryFindGroundZ(const FVector& TraceLocation, float CurrentZ, float& OutGroundZ) const;
	void UpdateHandSlots() const;
	void UpdateAudio(bool bActuallyMoved);
	void UpdateIK() const;
	
	void ApplySpeedMultiplierToOwner(const TObjectPtr<ACharacter> & Interactor, float Multiplier);
	UPushSpotComponent* GetNearestPushSpot(const FVector& FromLocation) const;
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	virtual void StartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor) override;
	
	virtual void Reset() override;
	virtual bool GetNearestInteractionSpotLocation(const FVector& FromLocation, FVector& OutLocation, FVector& OutDirection) override;
#pragma endregion
};
