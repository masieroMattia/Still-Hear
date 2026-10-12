#pragma once

#include "CoreMinimal.h"
#include "Interactions/Components/BaseTriggerComponent.h"

#include "GameplayTagContainer.h"
#include "TeleportTriggerComponent.generated.h"

class UCommonActivatableWidget;
class UUserWidget;

/**
 * Trigger component that teleports the player to a target location and rotation
 * Optionally displays a widget on screen for a specified duration
 */
UCLASS(ClassGroup=(Interactions), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UTeleportTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

public:
	UTeleportTriggerComponent();

#pragma region UPROPERTIES
protected:
	// Destination Actor in the level to teleport the player to (e.g. TargetPoint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Teleport")
	TSoftObjectPtr<AActor> DestinationPoint;

	// Time in seconds to delay the teleportation after trigger activation (useful to let a widget fade in)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Teleport", meta = (ClampMin = "0.0"))
	float TeleportDelay = 1.0f;

	// If true, displays a widget on the player's screen upon teleportation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Widget")
	bool bShowWidget = false;

	// The widget class to instantiate and display (must inherit from UCommonActivatableWidget)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Widget", meta = (EditCondition = "bShowWidget", EditConditionHides))
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	// Time in seconds to display the widget before hiding it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Widget", meta = (EditCondition = "bShowWidget", EditConditionHides, ClampMin = "0.0"))
	float WidgetDisplayDuration = 3.0f;

	// UI layer tag to push the widget to using the UI Subsystem
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Widget", meta = (EditCondition = "bShowWidget", EditConditionHides, Categories = "UI.Layer"))
	FGameplayTag WidgetLayerTag;
#pragma endregion

#pragma region VARIABLES
private:
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> ActiveWidget;
	
	FTimerHandle WidgetTimerHandle;
	FTimerHandle TeleportTimerHandle;
#pragma endregion

#pragma region METHODS
protected:
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

public:
	virtual void Reset() override;

private:
	UFUNCTION()
	void HideWidget();

	UFUNCTION()
	void ExecuteTeleport(AStillHearMainCharacter* Player, FVector DestLocation, FRotator DestRotation);
#pragma endregion
};
