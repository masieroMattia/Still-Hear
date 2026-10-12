#include "UI/Elements/RotatorBase.h"

#include "CommonInputSubsystem.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "UI/Elements/ButtonBase.h"

#pragma region CONSTRUCTOR
void URotatorBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonLeft)
		ButtonLeft->OnClicked().AddUObject(this, &URotatorBase::OnStepLeft);

	if (ButtonRight)
		ButtonRight->OnClicked().AddUObject(this, &URotatorBase::OnStepRight);

	UpdateVisuals();
}
#pragma endregion

#pragma region UFUNCTIONS
void URotatorBase::SetOptions(const TArray<FText>& InOptions)
{
	Options = InOptions;
	CurrentIndex = FMath::Clamp(CurrentIndex, 0, FMath::Max(0, Options.Num() - 1));
    
	UpdateVisuals();
}

void URotatorBase::SetSelectedIndex(const int32 Index)
{
	if (Options.IsEmpty())
		return;

	CurrentIndex = FMath::Clamp(Index, 0, Options.Num() - 1);
	
	UpdateVisuals();
}
#pragma endregion

#pragma region METHODS
void URotatorBase::OnStepLeft()
{
	if (Options.IsEmpty())
		return;

	CurrentIndex--;
    
	// Loop back to the end if we go below zero
	if (CurrentIndex < 0)
		CurrentIndex = Options.Num() - 1;

	UpdateVisuals();
	
	// Play value change animation
	if (StepLeftAnimation)
	{
		StopAnimation(StepLeftAnimation);
		PlayAnimation(StepLeftAnimation);
	}
	
	OnSelectionChanged.Broadcast(CurrentIndex, Options[CurrentIndex]);
}

void URotatorBase::OnStepRight()
{
	if (Options.IsEmpty())
		return;

	CurrentIndex++;
    
	// Loop back to the start if we exceed the array
	if (CurrentIndex >= Options.Num())
		CurrentIndex = 0;

	UpdateVisuals();
	
	// Play value change animation
	if (StepRightAnimation)
	{
		StopAnimation(StepRightAnimation);
		PlayAnimation(StepRightAnimation);
	}
	
	OnSelectionChanged.Broadcast(CurrentIndex, Options[CurrentIndex]);
}

void URotatorBase::UpdateVisuals()
{
	if (!TextValue || Options.IsEmpty())
		return;

	if (Options.IsValidIndex(CurrentIndex))
		TextValue->SetText(Options[CurrentIndex]);
}

void URotatorBase::RefreshFocusVisuals()
{
	Super::RefreshFocusVisuals();
	
	bool bIsGamepad = false;
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>())
			bIsGamepad = (InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad);
	}

	const bool bShouldBeFocused = bIsRowFocused && bIsGamepad;

	if (bShouldBeFocused && !bIsFocusAnimationActive)
	{
		PlayReversibleAnimation(FocusAnimation, EUMGSequencePlayMode::Forward);
		bIsFocusAnimationActive = true;
	}
	else if (!bShouldBeFocused && bIsFocusAnimationActive)
	{
		PlayReversibleAnimation(FocusAnimation, EUMGSequencePlayMode::Reverse);
		bIsFocusAnimationActive = false;
	}
}

void URotatorBase::PlayReversibleAnimation(UWidgetAnimation* InAnimation, const EUMGSequencePlayMode::Type PlayMode)
{
	if (!InAnimation) 
		return;

	float StartTime = 0.0f;

	if (IsAnimationPlaying(InAnimation))
	{
		StartTime = GetAnimationCurrentTime(InAnimation);
	}
	else if (PlayMode == EUMGSequencePlayMode::Reverse)
	{
		StartTime = InAnimation->GetEndTime();
	}

	StopAnimation(InAnimation);
	PlayAnimation(InAnimation, StartTime, 1, PlayMode);
}
#pragma endregion
