#pragma once

#include "CoreMinimal.h"
#include "Interactions/Components/BaseTriggerComponent.h"
#include "InteractableTriggerComponent.generated.h"

class AInteractableObj;

// Defines when the trigger fires relative to overlap events
UENUM(BlueprintType)
enum class ETriggerActivationEvent : uint8
{
	OnEnter			UMETA(DisplayName = "On Enter Only"),
	OnExit			UMETA(DisplayName = "On Exit Only"),
	OnEnterAndExit	UMETA(DisplayName = "On Enter and Exit")
};

UCLASS(ClassGroup=(Interactions), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UInteractableTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Interactable objects that will be triggered when the overlap event fires
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<TSoftObjectPtr<AInteractableObj>> InteractableObjects;

	// Determines whether the trigger fires on overlap begin, overlap end, or both
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	ETriggerActivationEvent TriggerActivation = ETriggerActivationEvent::OnEnter;
#pragma endregion

#pragma region METHODS
protected:
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

private:
	// Resolves soft references and calls TriggerInteraction on every valid interactable object
	void ActivateInteractables();
#pragma endregion
};