#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/PopupWidget.h"
#include "TutorialPopupWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UCommonActionWidget;
class UCommonTextBlock;
class UInputAction;
class UWrapBox;

/**
 * A popup widget that displays a tutorial message containing input actions
 * The message should contain {0}, {1}, etc. which will be replaced by the corresponding input action widget
 */
UCLASS()
class STILLHEAR_API UTutorialPopupWidget : public UPopupWidget
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Container for the generated text and input action widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> Container;

	// The widget class to spawn for text segments
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TSubclassOf<UCommonTextBlock> TextBlockClass;

	// The widget class to spawn for input actions
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TSubclassOf<class UInputActionWidget> InputActionWidgetClass;
#pragma endregion

#pragma region VARIABLES
private:

	struct FActionWidgetState
	{
		TObjectPtr<UUserWidget> Widget;
		TObjectPtr<UInputAction> Action;
		bool bWasPressed;
	};

	TArray<FActionWidgetState> TrackedActionWidgets;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Initializes the tutorial text and spawns the required input action widgets
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void InitializeTutorial(const FText& InMessage, const TArray<UInputAction*>& InputActions, float InDuration);
	
	// Blueprint Implementable Event to handle the assignment of the input action to the spawned widget 
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnInputActionWidgetSpawned(UUserWidget* SpawnedWidget, UInputAction* InputAction);
	
	// Called when the physical key mapped to the InputAction is pressed down
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnInputActionWidgetPressed(UUserWidget* ActionWidget, UInputAction* InputAction);

	// Called when the physical key mapped to the InputAction is released
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
	void OnInputActionWidgetReleased(UUserWidget* ActionWidget, UInputAction* InputAction);
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	// Polls the input subsystem and updates the state of all tracked action widgets
	void ProcessActionWidgetInput();

	// Updates the state and animations for a single action widget
	void UpdateActionWidgetState(FActionWidgetState& State, APlayerController* PC, UEnhancedInputLocalPlayerSubsystem* Subsystem);

	void AddTextSegment(const FString& TextSegment);
#pragma endregion
		
#pragma region INTERFACE METHODS
public:
	// IUIWidgetInterface implementation
	virtual void InitializeWidget_Implementation(const FText& Text, float Duration, const TArray<UInputAction*>& InputActions) override;
#pragma endregion
};
