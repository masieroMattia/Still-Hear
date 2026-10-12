#pragma once

#include "UI/UIEnum.h"
#include "GameplayTagContainer.h"
#include "CommonActivatableWidget.h"
#include "ActivatableWidgetBase.generated.h"

enum class E_WidgetInputMode : uint8;
class UCommonButtonBase;
class UButtonBase;

/*
 * Base class for all activatable UI widgets in the game
 * Provides functionality for input configuration and optional UI state tagging
 */

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UActivatableWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// Optional: A Gameplay Tag that represents the UI state this widget is associated with
	UPROPERTY(EditDefaultsOnly, Category = "Config|Tag")
	FGameplayTag UIStateTag;
	
	// The desired input mode to use while this UI is activated
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "UI.Layer"), Category = "Config|Input")
	E_WidgetInputMode InputConfig = E_WidgetInputMode::Default;
	
	// The desired mouse behavior when the game gets input
	UPROPERTY(EditDefaultsOnly, Category = "Config|Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
	
	// The data table row for the Back/Cancel action
	UPROPERTY(EditDefaultsOnly, Category = "Config|Input")
	FDataTableRowHandle BackInputAction;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> BackButton;
#pragma endregion
	
#pragma region VARIABLES
private:
	bool bBackActionBlocked = false;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> LastDisabledContainer;

	UPROPERTY(Transient)
	TObjectPtr<UButtonBase> LastClickedButton;
	
	// Handle to keep track of the registered back action
	FUIActionBindingHandle BackHandle;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	UFUNCTION()
	virtual void HandleBackActionBinding();
	
	// Overrides to provide custom input configuration for this widget
	// E.g., whether it consumes all input, allows game input, or shows a mouse cursor
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Routing")
	void RouteAction(FGameplayTag ActionTag, UButtonBase* ClickedButton);

	// Call this before triggering a transition to another widget to prevent back from firing mid-animation
	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void SetBackActionBlocked(bool bBlocked);

	// Returns the widget that should receive focus when this widget becomes active
	// Override in subclasses to point deeper into the widget hierarchy
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Focus")
	UWidget* GetPreferredFocusTarget() const;
#pragma endregion
};