#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "SaveSystem/Savable.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Restorable.h"
#include "Interfaces/Interactable.h"
#include "Components/MeshComponent.h"
#include "GameplayTagAssetInterface.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "InteractableObj.generated.h"

class USplineVFXTravelerComponent;
class UInteractableShakeComponent;
class UIndicatorComponent;
class UAudioComponent;
class UInputAction;

USTRUCT(BlueprintType)
struct FSplineVFXTravelerTrigger
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReverse = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggeredBy, AActor*, Triggerer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStopped);

UCLASS()
class STILLHEAR_API AInteractableObj : public AActor, public IInteractable, public IGameplayTagAssetInterface, public IRestorable, public ISavable
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USaveIdComponent> SaveIdComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UNiagaraComponent> InteractNiagaraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProximityNiagaraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UIndicatorComponent> IndicatorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractableShakeComponent> ShakeComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Overlap Config", meta = (Categories = "Interaction"))
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Tags", meta = (Categories = "Interaction"))
	FGameplayTagContainer InteractableTags; // Gameplay tags to identify the interactable object
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Reset")
	bool bResetObj = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bDestroyOnEnd = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Dissolve", meta = (EditCondition = "bDestroyOnEnd", EditConditionHides))
	TObjectPtr<UCurveFloat> DissolveCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Dissolve", meta = (EditCondition = "bDestroyOnEnd", EditConditionHides))
	FName DissolveParameterName = "FadeOpacity";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Dissolve", meta = (EditCondition = "bDestroyOnEnd", EditConditionHides))
	TArray<FComponentReference> DissolveTargets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Dissolve", meta = (EditCondition = "bDestroyOnEnd", EditConditionHides))
	bool bDestroyActorOnDissolveFinished = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable Settings|Highlight")
	UMaterialInterface* HighlightMaterial; // Material used to highlight the object when it is interactable
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> StartInteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> EndInteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> ProximitySound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	float FadeInDuration = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	float FadeOutDuration = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> StartInteractionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> EndInteractionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> ProximityVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TArray<FSplineVFXTravelerTrigger> LinkedVFXTravelers;
#pragma endregion
	
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Interactable Settings|Events")
	FOnInteractionStarted OnInteractionStarted;
	UPROPERTY(BlueprintAssignable, Category = "Interactable Settings|Events")
	FOnInteractionStopped OnInteractionStopped;
	UPROPERTY(BlueprintAssignable, Category = "Interactable Settings|Events")
	FOnTriggeredBy OnTriggeredBy;
#pragma endregion
	
#pragma region VARIABLES
	FGameplayTagContainer OriginalInteractableTags;
	TArray<TObjectPtr<UInputAction>> OriginalPromptActions;
	TSubclassOf<UUserWidget> OriginalSeparatorClass;
	
	FVector OriginalLocation;
	FRotator OriginalRotation;

	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	FVector CheckpointLocation;
	UPROPERTY(SaveGame)
	FRotator CheckpointRotation;
	UPROPERTY(SaveGame)
	bool bCheckpointIsConsumed = false;
	UPROPERTY(SaveGame)
	bool bCheckpointIsInteracting = false;
	UPROPERTY(SaveGame)
	bool bCheckpointIsHidden = false;
	UPROPERTY(SaveGame)
	FGameplayTagContainer CheckpointInteractableTags;

	bool bIsInteracting = false;
	bool bIsConsumed = false;
	bool bIsTriggeredByChain = false;
	
	FTimeline DissolveTimeline;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMeshComponent>> ResolvedDissolveMeshes;
	
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> StartInteractionAudioComponent;
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> EndInteractionAudioComponent;
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ProximityAudioComponent;
	
	FTimerHandle PreInteractionTimerHandle;
	UPROPERTY()
	TObjectPtr<ACharacter> PendingInteractor;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AInteractableObj();
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
	void PlayProximitySound(AActor* OtherActor);
	void StopProximitySound(AActor* OtherActor);
	
	void PlayProximityVFX(AActor* OtherActor);
	void StopProximityVFX(AActor* OtherActor);

public:
	virtual void Tick(float DeltaTime) override;
#pragma endregion
	
#pragma region UFUNCTIONS
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(BlueprintCallable, Category="Interactable") 
	void CallStartInteraction();
	UFUNCTION(BlueprintCallable, Category="Interactable")
	void CallEndInteraction();
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	virtual void TriggerInteraction(AActor* Triggerer = nullptr) override;

	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor = nullptr);

	UFUNCTION()
	void HandleDissolveUpdate(float Value);
	UFUNCTION()
	void HandleDissolveFinished();
	
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void SetHighlight(const bool bEnableHighlight) const;
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	virtual void StartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	
	bool IsInteracting() const { return bIsInteracting; }
	
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
	virtual void OnPostLoad_Implementation() override;
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
#pragma endregion
};
