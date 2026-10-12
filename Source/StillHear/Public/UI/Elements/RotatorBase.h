#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "RotatorBase.generated.h"

enum class ECommonInputType : uint8;
class UButtonBase;
class UTextBlock;

// Delegate to broadcast when the user cycles the rotator
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRotatorSelectionChanged, int32, SelectedIndex, FText, SelectedText);

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API URotatorBase : public USettingsRowBase
{
	GENERATED_BODY()

protected:
#pragma region UPROPERTIES
	// The available options to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotator")
	TArray<FText> Options;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rotator")
	int32 CurrentIndex = 0;

	// Required UI bindings
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> ButtonLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> ButtonRight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TextValue;
	
	// Animation played when the row receives gamepad focus
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FocusAnimation;

	// Animation played when stepping Left
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> StepLeftAnimation;

	// Animation played when stepping Right
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> StepRightAnimation;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Rotator")
	FOnRotatorSelectionChanged OnSelectionChanged;
#pragma endregion

#pragma region VARIABLES
	bool bIsFocusAnimationActive = false;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region UFUNCTIONS
	// Populates the rotator with text options
	UFUNCTION(BlueprintCallable, Category = "Rotator")
	void SetOptions(const TArray<FText>& InOptions);

	// Forces the rotator to display a specific index
	UFUNCTION(BlueprintCallable, Category = "Rotator")
	void SetSelectedIndex(int32 Index);

	// Returns the currently selected index
	UFUNCTION(BlueprintPure, Category = "Rotator")
	int32 GetSelectedIndex() const { return CurrentIndex; }
#pragma endregion

#pragma region METHODS
protected:
	virtual void OnStepLeft() override;
	virtual void OnStepRight() override;
	
	void UpdateVisuals();

private:
	virtual void RefreshFocusVisuals() override;

	void PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode);
#pragma endregion
};