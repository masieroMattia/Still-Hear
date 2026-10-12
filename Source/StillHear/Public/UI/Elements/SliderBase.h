#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "SliderBase.generated.h"

class UTextBlock;
class USlider;
class UProgressBar;

// Delegate to broadcast when the slider is moved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliderValueChanged, float, NewValue);

UCLASS()
class STILLHEAR_API USliderBase : public USettingsRowBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "Slider")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", UIMin = "1.0"),Category = "Slider")
	float MaxValue = 1.0f;
	
	// How much the slider moves when using gamepad D-pad or stick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
	float NavigationStep = 0.1f;
	
	// Controls how many decimals to show in the text block
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
	int32 FractionalDigits = 1;
	
	// Color when the slider is at its minimum value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Style")
	FLinearColor MinValueColor = FLinearColor::Green;

	// Color when the slider is at its maximum value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Style")
	FLinearColor MaxValueColor = FLinearColor::Red;
	
	// Left image (fill) when normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Style")
	FSlateBrush NormalFillImage;

	// Left image (fill) when selected/hovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Style")
	FSlateBrush SelectedFillImage;

	// Right image (background)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Style")
	FSlateBrush BackgroundBarImage;
	
	// Optional progress bar used to render the fill effect
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> FillProgressBar;
	
	// Required UI bindings
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> MainSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TextValue;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HoveredAnimation;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> GrabbedAnimation;
#pragma endregion
    
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Slider")
	FOnSliderValueChanged OnValueChanged;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region UFUNCTIONS
	// Sets the slider value externally
	UFUNCTION(BlueprintCallable, Category = "Slider")
	void SetValue(const float InValue);

	// Returns the current slider value
	UFUNCTION(BlueprintPure, Category = "Slider")
	float GetValue() const { return CurrentValue; }
#pragma endregion

#pragma region METHODS
protected:
	UFUNCTION()
	virtual void HandleSliderChanged(const float NewValue);
	UFUNCTION()
	void HandleGrabBegin();
	UFUNCTION()
	void HandleGrabEnd();
	
	virtual void OnStepLeft() override;
	virtual void OnStepRight() override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	
	void PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode);
	void UpdateVisuals() const;
	void UpdateProgressBarVisuals();
#pragma endregion
};