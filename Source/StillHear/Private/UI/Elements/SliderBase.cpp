#include "UI/Elements/SliderBase.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#pragma region CONSTRUCTOR
void USliderBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainSlider)
	{
		MainSlider->SetMinValue(MinValue);
		MainSlider->SetMaxValue(MaxValue);

		// RemoveDynamic before AddDynamic to prevent duplicate bindings
		// (NativeConstruct is called every time the widget is pushed to the layer)
		MainSlider->OnValueChanged.RemoveDynamic(this, &USliderBase::HandleSliderChanged);
		MainSlider->OnValueChanged.AddDynamic(this, &USliderBase::HandleSliderChanged);

		MainSlider->OnMouseCaptureBegin.RemoveDynamic(this, &USliderBase::HandleGrabBegin);
		MainSlider->OnMouseCaptureBegin.AddDynamic(this, &USliderBase::HandleGrabBegin);

		MainSlider->OnMouseCaptureEnd.RemoveDynamic(this, &USliderBase::HandleGrabEnd);
		MainSlider->OnMouseCaptureEnd.AddDynamic(this, &USliderBase::HandleGrabEnd);
		
		MainSlider->IsFocusable = false;
	}

	UpdateVisuals();
	UpdateProgressBarVisuals();
}
#pragma endregion

#pragma region UFUNCTIONS
void USliderBase::SetValue(const float InValue)
{
	CurrentValue = InValue;

	if (MainSlider)
		MainSlider->SetValue(CurrentValue);

	UpdateVisuals();
}
#pragma endregion

#pragma region METHODS
void USliderBase::HandleSliderChanged(const float NewValue)
{
	CurrentValue = NewValue;
    
	UpdateVisuals();
	OnValueChanged.Broadcast(CurrentValue);
}

void USliderBase::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	
	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Forward);
	UpdateProgressBarVisuals();
}

void USliderBase::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Reverse);
	UpdateProgressBarVisuals();
}

void USliderBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	
	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Forward);
	UpdateProgressBarVisuals();
}

void USliderBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	
	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Reverse);
	UpdateProgressBarVisuals();
}

void USliderBase::HandleGrabBegin()
{
	PlayReversibleAnimation(GrabbedAnimation, EUMGSequencePlayMode::Forward);
}

void USliderBase::HandleGrabEnd()
{
	PlayReversibleAnimation(GrabbedAnimation, EUMGSequencePlayMode::Reverse);
}

void USliderBase::OnStepLeft()
{
	const float NewValue = FMath::Clamp(CurrentValue - NavigationStep, MinValue, MaxValue);
	
	SetValue(NewValue);
	OnValueChanged.Broadcast(CurrentValue);
}

void USliderBase::OnStepRight()
{
	const float NewValue = FMath::Clamp(CurrentValue + NavigationStep, MinValue, MaxValue);
	
	SetValue(NewValue);
	OnValueChanged.Broadcast(CurrentValue);
}

void USliderBase::PlayReversibleAnimation(UWidgetAnimation* InAnimation, const EUMGSequencePlayMode::Type PlayMode)
{
	if (!InAnimation) 
		return;

	float StartTime = 0.0f;

	// Cache the exact time if the animation is currently running to prevent jittering
	if (IsAnimationPlaying(InAnimation))
		StartTime = GetAnimationCurrentTime(InAnimation);
	else if (PlayMode == EUMGSequencePlayMode::Reverse)
		StartTime = InAnimation->GetEndTime();

	// Stop the animation to clear the evaluation buffer before restarting it
	StopAnimation(InAnimation);

	// Play from the calculated time
	PlayAnimation(InAnimation, StartTime, 1, PlayMode);
}

void USliderBase::UpdateVisuals() const
{
	if (!TextValue || !MainSlider) 
		return;

	// Format the number to keep the UI clean based on designer settings
	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = FractionalDigits;
	FormatOptions.MaximumFractionalDigits = FractionalDigits;

	TextValue->SetText(FText::AsNumber(CurrentValue, &FormatOptions));

	const float Alpha = (MaxValue > MinValue) ? (CurrentValue - MinValue) / (MaxValue - MinValue) : 0.0f;
	const FLinearColor FinalColor = FMath::Lerp(MinValueColor, MaxValueColor, Alpha);

	MainSlider->SetSliderHandleColor(FinalColor);
	
	if (FillProgressBar)
	{
		FillProgressBar->SetPercent(Alpha);
	}
}

void USliderBase::UpdateProgressBarVisuals()
{
	if (!FillProgressBar)
		return;

	const bool bIsSelected = IsHovered() || HasAnyUserFocus();

	FProgressBarStyle Style = FillProgressBar->GetWidgetStyle();
	Style.FillImage = bIsSelected ? SelectedFillImage : NormalFillImage;
	Style.BackgroundImage = BackgroundBarImage;
	
	FillProgressBar->SetWidgetStyle(Style);
}
#pragma endregion