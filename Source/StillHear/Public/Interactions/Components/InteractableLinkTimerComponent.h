#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Savable.h"
#include "Interfaces/Restorable.h"
#include "Components/ActorComponent.h"
#include "InteractableLinkTimerComponent.generated.h"

class AInteractableObj;
class ATargetableDrivableObj;

UENUM(BlueprintType)
enum class ELinkTriggerType : uint8
{
	Any           UMETA(DisplayName = "Any"),
	Interaction   UMETA(DisplayName = "Interaction Only"),
	Hit           UMETA(DisplayName = "Hit Only")
};

USTRUCT(BlueprintType)
struct FInteractableLinkEntry
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	TSoftObjectPtr<AInteractableObj> Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emissive")
	FComponentReference EmissiveMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emissive", meta = (ClampMin = 0, UIMin = 0))
	int32 EmissiveMaterialIndex = 0;
	// Target scalar value the emissive interpolates to once activated
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emissive")
	float EmissiveTargetValue = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer")
	FComponentReference TimerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (ClampMin = 0, UIMin = 0))
	int32 TimerMaterialIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	ELinkTriggerType TriggerType = ELinkTriggerType::Hit;
};

USTRUCT(BlueprintType)
struct FMaterialArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Mats;
};

UCLASS(ClassGroup=(Interactions), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UInteractableLinkTimerComponent : public UActorComponent, public IRestorable, public ISavable
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<FInteractableLinkEntry> LinkedSources;

	// Seconds before the whole sequence resets (restarts on every new event)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Timeout = 5.0f;

	// Scalar parameter name for the emissive material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName EmissiveParameterName = "EmissiveMultiplier";

	// Scalar parameter name for the timer material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName TimerParameterName = "TimerProgress";

	// How fast the emissive material interpolates to its target value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float EmissiveInterpSpeed = 5.0f;
#pragma endregion

#pragma region VARIABLES
private:
	TArray<bool> ActivatedFlags;
	TArray<float> EmissiveCurrentValues;
	TArray<float> EmissiveTargetValues;
	UPROPERTY()
	TArray<float> TimerCurrentValues;
	FTimerHandle TimeoutHandle;
	float TimerStartTime = 0.0f;
	UPROPERTY()
	bool bIsCompleted = false;

	UPROPERTY()
	TArray<TObjectPtr<AInteractableObj>> ResolvedSources;
	UPROPERTY()
	TArray<FMaterialArrayWrapper> EmissiveMaterials;
	UPROPERTY()
	TArray<FMaterialArrayWrapper> TimerMaterials;

	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasTimerCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	TArray<bool> CheckpointActivatedFlags;
	UPROPERTY(SaveGame)
	bool bCheckpointIsCompleted = false;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UInteractableLinkTimerComponent();
#pragma endregion

#pragma region METHODS
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
	virtual void OnPostLoad_Implementation() override;

private:
	UFUNCTION()
	void OnAnySourceTriggered();

	void OnSourceTriggered(int32 SourceIndex);
	void OnTimeout();
	bool AreAllSourcesActive() const;
	void ResetAllSources();
	void StartTimer();
	void StopTimer();
#pragma endregion
};
