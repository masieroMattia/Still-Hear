#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "SaveSystem/Savable.h"
#include "GameplayTagContainer.h"
#include "Interfaces/Restorable.h"
#include "Chaos/CacheCollection.h"
#include "Interfaces/Interactable.h"
#include "Chaos/CacheManagerActor.h"
#include "GameplayTagAssetInterface.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Interactions/Components/InteractableShakeComponent.h"
#include "ChaosResonanceObj.generated.h"

class UInputAction;
class UAudioComponent;
class UIndicatorComponent;
class UInteractableShakeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChaosTriggeredBy, AActor*, Triggerer);

UENUM(BlueprintType)
enum class EChaosResonanceState : uint8
{
	Intact    UMETA(DisplayName = "Intact"),
	Breaking  UMETA(DisplayName = "Breaking"),
	Consumed  UMETA(DisplayName = "Consumed"),
};

UCLASS()
class STILLHEAR_API AChaosResonanceObj : public AChaosCachePlayer, public IInteractable, public IGameplayTagAssetInterface, public IRestorable, public ISavable
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USaveIdComponent> SaveIdComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> CoreGeometryCollectionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGeometryCollectionComponent> DebrisGeometryCollectionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> OutlineMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UIndicatorComponent> IndicatorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractableShakeComponent> ShakeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProximityNiagaraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings", meta = (Categories = "Interaction"))
	FGameplayTagContainer ResonanceTags; // Gameplay tags to identify the resonance object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings|Overlap Config", meta = (Categories = "Interaction"))
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Resonance Settings|Chaos Setup")
	TObjectPtr<UChaosCacheCollection> CacheColl;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Resonance Settings|Chaos Setup")
	FName CoreCacheTrackName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Resonance Settings|Chaos Setup")
	FName DebrisCacheTrackName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resonance Settings|Timing")
	TObjectPtr<UCurveFloat> FadeCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> ChaosInteractionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	TObjectPtr<USoundBase> ProximitySound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	float FadeInDuration = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Sound")
	float FadeOutDuration = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings", meta = (Categories = "Event"))
	FGameplayTag InteractionSuccessTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	bool bReset = true;

	// Whether the player can climb this object (adds the "Climb" actor tag on spawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	bool bClimbable = false;

	// Optional linked resonance object that triggers automatically when this object starts its interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Settings")
	TObjectPtr<AChaosResonanceObj> LinkedResonanceObj;
	
	// Actions to display in the prompt while the object is resonating
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resonance Settings|Input")
	TArray<TObjectPtr<UInputAction>> ResonancePromptActions;

	// Custom separator to display in the prompt while the object is resonating
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resonance Settings|Input")
	TSubclassOf<UUserWidget> ResonancePromptSeparatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|VFX")
	TObjectPtr<UNiagaraSystem> ProximityVFX;
#pragma endregion
	
#pragma region EVENTS
	UPROPERTY(BlueprintAssignable, Category = "Resonance Settings|Events")
	FOnChaosTriggeredBy OnTriggeredBy;
#pragma endregion
	
#pragma region VARIABLES
protected:
	FTimeline DebrisTimeline;
	FTimerHandle PreInteractionTimerHandle;

	FTransform InitialTransform = FTransform::Identity;

	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	bool bCheckpointIsConsumed = false;
	
	UPROPERTY()
	TObjectPtr<ACharacter> PendingInteractor;

	bool bIsInteracting = false;
	UPROPERTY(SaveGame)
	bool bIsConsumed = false;

	bool bCloneSpawnPending = false; // Guard to prevent duplicate next-tick clone spawns
	bool bHasCoreGeometry = false;   // Cached on BeginPlay: true if the core geometry component has a valid asset

	// Stores the actor that triggered this object via TriggerInteraction, forwarded to OnTriggeredBy in ExecuteStartInteraction
	UPROPERTY()
	TObjectPtr<AActor> PendingTriggerer;

	TArray<TObjectPtr<UInputAction>> OriginalPromptActions;
	TSubclassOf<UUserWidget> OriginalSeparatorClass;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ChaosAudioComponent;
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ProximityAudioComponent;
#pragma endregion

#pragma region CONSTRUCTOR
	AChaosResonanceObj();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void DeferredRespawn(const FTransform& SpawnTransform);
public:
	virtual void Tick(float DeltaSeconds) override;
	void SetHighlight(const bool bEnableHighlight) const;

	void PlayProximityVFX(AActor* OtherActor);
	void StopProximityVFX(AActor* OtherActor);
	
	// Transitions the actor into its consumed state, disabling logic and preserving only core geometry if present
	void ApplyConsumedState();
#pragma endregion
	
#pragma region UFUNCTIONS
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void HandleFadeUpdate(float Value) const;
	UFUNCTION()
	void HandleFadeFinished();
	
	// Interactor is optional: pass it when this trigger should also make the interactor ignore the
	// Resonance collision channel during the break. Leave it unset for triggers that should NOT touch the interactor's collision at all.
	// CollisionIgnoreDelay (seconds, default 0 = immediate) delays making the interactor ignore the Resonance collision
	UFUNCTION(BlueprintCallable, Category="ChaosResonance")
	void CallInteraction(ACharacter* Interactor = nullptr, float CollisionIgnoreDelay = 0.0f);

	UFUNCTION(BlueprintPure, Category = "ChaosResonance")
	EChaosResonanceState GetResonanceState() const;
	UFUNCTION(BlueprintCallable, Category = "ChaosResonance")
	virtual void TriggerInteraction(AActor* Triggerer = nullptr) override;

	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor = nullptr, float CollisionIgnoreDelay = 0.0f);

	// The clone spawned to bypass streaming bugs. The original actor forwards interactions to this clone
	UPROPERTY(BlueprintReadOnly, Category = "Resonance")
	TObjectPtr<AChaosResonanceObj> ActiveClone;

	// The proxy that spawned this clone. Used to update the proxy when this clone replaces itself on reset
	UPROPERTY(BlueprintReadOnly, Category = "Resonance")
	TObjectPtr<AChaosResonanceObj> MyProxy;
#pragma endregion
	
#pragma region INTERFACE METHODS
public: 
	virtual void StartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
	virtual void OnPostLoad_Implementation() override;
#pragma endregion
};