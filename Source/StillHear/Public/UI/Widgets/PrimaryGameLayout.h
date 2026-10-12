#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "PrimaryGameLayout.generated.h"

class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;

UCLASS(Abstract)
class STILLHEAR_API UPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// Map connecting gameplay tags to their physical stack containers
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> Layers;
#pragma endregion
	
#pragma region VARIABLES
	UPROPERTY(Transient)
	TSet<TObjectPtr<UCommonActivatableWidget>> PausingWidgets;
	
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Registers a stack widget to a specific UI layer tag
	UFUNCTION(BlueprintCallable, meta = (Categories = "UI.Layer"), Category = "UI")
	void RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetStack* LayerWidget);

	// Pushes a new widget to the specified layer
	UFUNCTION(BlueprintCallable, meta = (Categories = "UI.Layer"), Category = "UI")
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, const bool bClearLayer = false, const bool bPauseGame = false);

	// Clears all widgets from the specified layer
	UFUNCTION(BlueprintCallable, meta = (Categories = "UI.Layer"), Category = "UI")
	void ClearLayer(FGameplayTag LayerTag);
#pragma endregion

#pragma region METHODS
private:
	UFUNCTION()
	void OnPausingWidgetDeactivated(UCommonActivatableWidget* DeactivatedWidget);
#pragma endregion
};