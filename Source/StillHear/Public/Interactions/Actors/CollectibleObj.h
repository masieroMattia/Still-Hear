#pragma once

#include "CoreMinimal.h"
#include "InteractableObj.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CollectibleObj.generated.h"

class ULevelSequencePlayer;
class ALevelSequenceActor;
class UImagePopupWidget;
class ULevelSequence;

// What happens when the player collects this collectible
UENUM(BlueprintType)
enum class ECollectibleBehavior : uint8
{
	PlayCinematic          UMETA(DisplayName = "Play Cinematic"),              // Trigger a Level Sequence, then destroy
	ShowUIWidget           UMETA(DisplayName = "Show UI Widget"),              // Push a widget to the UI layer, then destroy
	PlayCinematicThenUI    UMETA(DisplayName = "Play Cinematic Then Show UI"), // Play the cinematic; when it ends, push the widget
};

UCLASS()
class STILLHEAR_API ACollectibleObj : public AInteractableObj
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;

	UPROPERTY(SaveGame)
	bool bIsCollected = false;

	// ===== COLLECTIBLE BEHAVIOR =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	ECollectibleBehavior CollectibleBehavior = ECollectibleBehavior::PlayCinematic;

	// Level Sequence Actor already placed in the level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config", meta = (EditCondition = "CollectibleBehavior == ECollectibleBehavior::PlayCinematic || CollectibleBehavior == ECollectibleBehavior::PlayCinematicThenUI", EditConditionHides))
	TObjectPtr<ALevelSequenceActor> CollectibleCinematicActor;

	// Widget class to push
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config", meta = (EditCondition = "CollectibleBehavior == ECollectibleBehavior::ShowUIWidget || CollectibleBehavior == ECollectibleBehavior::PlayCinematicThenUI", EditConditionHides))
	TSubclassOf<UImagePopupWidget> CollectibleWidgetClass;

	// Data to pass to the widget / system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config", meta = (RowType = "/Script/StillHear.CollectibleData"))
	FDataTableRowHandle CollectibleDataRow;

	// UI layer where the widget will be pushed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config", meta = (EditCondition = "CollectibleBehavior == ECollectibleBehavior::ShowUIWidget || CollectibleBehavior == ECollectibleBehavior::PlayCinematicThenUI", EditConditionHides, Categories = "UI.Layer"))
	FGameplayTag CollectibleWidgetLayerTag;

	// Actors that should be activated (e.g. Niagara VFX Actors) if this collectible was already collected when loaded.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	TArray<TObjectPtr<AActor>> ActorsToActivateOnLoad;
#pragma endregion

#pragma region VARIABLES
	// Kept so we can explicitly unbind OnFinished and stop the player on EndPlay
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> CachedSequencePlayer;

	// Interactor that started this interaction
	UPROPERTY()
	TObjectPtr<ACharacter> CachedInteractor;
#pragma endregion

#pragma region CONSTRUCTOR
	ACollectibleObj();
#pragma endregion 

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Collectible")
	void OnCollectibleLoadedCollected();

private:
	void TriggerCinematic();
	void TriggerUIWidget();
	void OnContentFinished();
#pragma endregion
	
#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void OnCinematicFinished();
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor) override;

	// ISavable implementation
	virtual void OnPreSave_Implementation() override;
	virtual void OnPostLoad_Implementation() override;
#pragma endregion
};
