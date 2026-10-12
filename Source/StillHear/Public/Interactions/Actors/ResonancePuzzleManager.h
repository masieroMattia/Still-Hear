#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResonancePuzzleManager.generated.h"

class AResonancePuzzleManager;
class UAudioComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleFailed);

// Visual states for the pattern steps
UENUM(BlueprintType)
enum class EStepVisualState : uint8
{
	Off,
	Flashing,
	Success,
	Error
};

// Configurable step of the metronome cycle
USTRUCT(BlueprintType)
struct FMetronomeStepConfig
{
	GENERATED_BODY()

	// The static mesh components that light up in this step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metronome")
	TArray<FComponentReference> Gems;

	// Duration of this step in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metronome", meta = (ClampMin = "0.1"))
	float Duration = 1.0f;

	// Sound played when this step is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metronome")
	TObjectPtr<USoundBase> StepSound;

	// True if the player is allowed to resonate during this step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metronome")
	bool bIsResonanceWindow = false;
};

// Configuration for a single puzzle pattern step
USTRUCT(BlueprintType)
struct FPuzzleStepConfig
{
	GENERATED_BODY()

	// The visual static mesh  that represents this step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Step")
	FComponentReference StepVisualMesh;

	// True if this is a pause step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Step")
	bool bIsPauseStep = false;

	// The bell actor/component expected to be resonated in this step (disabled if pause step)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Step", meta = (EditCondition = "!bIsPauseStep", EditConditionHides = "!bIsPauseStep"))
	FComponentReference ExpectedBell;
};

// Resolved step data tracked internally at runtime
USTRUCT()
struct FResolvedPuzzleStep
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> VisualMesh = nullptr;

	UPROPERTY()
	bool bIsPauseStep = false;

	UPROPERTY()
	TObjectPtr<AActor> ExpectedBellActor = nullptr;
};

/**
 * Helper class that listens to individual bell resonance events and forwards them with the bell reference
 */
UCLASS()
class STILLHEAR_API UPuzzleBellListener : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TWeakObjectPtr<AActor> BellActor;
	UPROPERTY()
	TWeakObjectPtr<AResonancePuzzleManager> Manager;

	UFUNCTION()
	void OnInteractableStarted();
	UFUNCTION()
	void OnChaosTriggered(AActor* Triggerer);
};

/**
 * Manager class that coordinates the metronome, the bells, and the pattern symbols
 */
UCLASS()
class STILLHEAR_API AResonancePuzzleManager : public AActor
{
	GENERATED_BODY()

public:
	AResonancePuzzleManager();

#pragma region UPROPERTIES
public:
	// The sequence of steps for the metronome cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Metronome")
	TArray<FMetronomeStepConfig> MetronomeSteps;

	// The sequence of steps for the puzzle solution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Config")
	TArray<FPuzzleStepConfig> PuzzleSteps;

	// Optional list of interactable actors to trigger when the puzzle is solved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Config")
	TArray<TObjectPtr<AActor>> SuccessTargetActors;

	// Components to illuminate with emissive glow when the puzzle is solved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	TArray<FComponentReference> SuccessGlowComponents;

	// Material emissive parameter name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FName EmissiveParamName = TEXT("GlowIntensity");

	// Material color parameter name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FName EmissiveColorParamName = TEXT("GlowColor");

	// Active step emissive glow intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	float ActiveGlowIntensity = 5.0f;

	// Speed to blend the metronome glow intensities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	float GlowBlendSpeed = 8.0f;

	// Speed of the flashing active pattern steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	float FlashFrequency = 5.0f;

	// Minimum glow intensity during flashing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	float MinFlashGlowIntensity = 0.5f;

	// Colors for pattern steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FLinearColor ActiveColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange/Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FLinearColor SuccessColor = FLinearColor(0.0f, 1.0f, 0.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FLinearColor ErrorColor = FLinearColor(1.0f, 0.0f, 0.0f); // Red

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Aesthetics")
	FLinearColor InactiveColor = FLinearColor(0.0f, 0.0f, 0.0f); // Black/Off

	// Duration of the error red flashing feedback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Timing")
	float ErrorDuration = 2.0f;

	// Sound played when a single step is completed successfully
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Sounds")
	TObjectPtr<USoundBase> StepSuccessSound;

	// Sound played when the puzzle is solved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Sounds")
	TObjectPtr<USoundBase> PuzzleSuccessSound;

	// Sound played when player makes an error
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Sounds")
	TObjectPtr<USoundBase> PuzzleFailureSound;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Puzzle|Events")
	FOnPuzzleSolved OnPuzzleSolved;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle|Events")
	FOnPuzzleFailed OnPuzzleFailed;
#pragma endregion

#pragma region COMPONENTS
private:
	// Pre-allocated audio components for spatialized audio and optimization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> MetronomeAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> FeedbackAudioComponent;

	// Action area trigger box where bells must be located for the puzzle to start/run
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ActionAreaCollider;
#pragma endregion

#pragma region RUNTIME_STATE
private:
	// Metronome runtime state
	int32 CurrentMetronomeStepIndex = 0;
	float StepTimer = 0.0f;

	// Puzzle progression state
	int32 CurrentPatternStepIndex = 0;
	bool bStepCompletedThisCycle = false;
	bool bInErrorState = false;
	bool bPuzzleSolved = false;

	// Puzzle active state (true when all expected bells are inside the action area)
	bool bPuzzleActive = false;

	// Set of bells currently inside the action area
	UPROPERTY()
	TSet<TObjectPtr<AActor>> BellsInsideArea;
#pragma endregion

#pragma region CACHED_REFERENCES
private:
	// Resolved components and step configs
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ResolvedMetronomeGems;
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ResolvedSuccessGlowComponents;
	UPROPERTY()
	TArray<FResolvedPuzzleStep> ResolvedPuzzleSteps;
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ResolvedPatternStepMeshes;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ResolvedBells;

	// Listener objects to prevent garbage collection
	UPROPERTY()
	TArray<TObjectPtr<UPuzzleBellListener>> BellListeners;
#pragma endregion

#pragma region MATERIAL_INSTANCES
private:
	// Materials instances mapping to avoid lookup and recreate overhead
	UPROPERTY()
	TMap<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>> MetronomeGemMIDs;

	UPROPERTY()
	TMap<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>> PatternStepMIDs;

	UPROPERTY()
	TMap<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>> SuccessGlowMIDs;

	// Map to track current state of each pattern step visual
	TMap<TObjectPtr<UStaticMeshComponent>, EStepVisualState> PatternStepVisualStates;

	// Caches to optimize dynamic material updates (avoiding GetScalarParameterValue and redundant Set calls)
	TMap<TObjectPtr<UStaticMeshComponent>, float> MetronomeGemIntensities;
	TMap<TObjectPtr<UStaticMeshComponent>, float> PatternStepIntensities;
	TMap<TObjectPtr<UStaticMeshComponent>, FLinearColor> PatternStepColors;
	TMap<TObjectPtr<UStaticMeshComponent>, float> SuccessGlowIntensities;
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void HandleBellResonance(AActor* RungBell);

private:
	// Initialization and Caching
	void ResolveAllReferences();
	void CreateDynamicMaterials();
	void BindToBells();

	// Metronome Tick updates
	void UpdateMetronome(float DeltaTime);
	void OnMetronomeStepTransition(int32 OldStepIndex, int32 NewStepIndex);
	void UpdateGemMaterials(float DeltaTime);

	// Pattern steps Tick updates
	void UpdatePatternStepAnimations();
	void UpdateSuccessGlow(float DeltaTime);

	// Puzzle state progression
	void OnStepSucceeded(int32 StepIndex);
	void TriggerFailure(bool bIsTimeout = false);
	void TriggerSuccess();
	
	UFUNCTION()
	void ResetPuzzleAfterError();

	// Action Area Overlap Checkers and Handlers
	void CheckInitialBellsOverlapping();
	void CheckInitialBellsAndStart();
	bool AreAllBellsInside() const;
	void StartPuzzle();
	void InitializePuzzleDormant();

	UFUNCTION()
	void OnActionAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnActionAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Helpers
	UStaticMeshComponent* ResolveComponentRef(const FComponentReference& Ref) const;
	void SetPatternStepVisualState(int32 StepIndex, EStepVisualState NewState);
#pragma endregion
};
