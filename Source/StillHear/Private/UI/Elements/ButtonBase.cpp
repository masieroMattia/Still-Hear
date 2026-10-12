#include "UI/Elements/ButtonBase.h"

#include "Engine/World.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "CommonActionWidget.h"
#include "UObject/UnrealType.h"
#include "CommonInputSubsystem.h"
#include "Animation/WidgetAnimation.h"
#include "UI/Widgets/ActivatableWidgetBase.h"

#pragma region METHODS
void UButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateButtonStyle();
	RefreshButtonContent();
}

void UButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	OnClicked().AddUObject(this, &UButtonBase::HandleActionClick);
	
	if (const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		LastInputType = InputSubsystem->GetCurrentInputType();
		
		if (bDisableFocusOnGamepad)
		{
			const bool bIsMouseAndKeyboard = (LastInputType == ECommonInputType::MouseAndKeyboard);
			SetIsFocusable(bIsMouseAndKeyboard);
		}
		else
		{
			SetIsFocusable(true);
		}
	}
	else
	{
		SetIsFocusable(true);
	}

	UpdateFocusIndicator();
}

void UButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	if (const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		LastInputType = InputSubsystem->GetCurrentInputType();
	}

	UpdateButtonStyle();
	RefreshButtonContent();
}

void UButtonBase::RefreshButtonContent() const
{
	if (!GetOwningLocalPlayer())
		return; 
	
	if (InputActionWidget)
	{
		bool bShouldShow = bShowInputAction;
		if (bShouldShow && bShowInputActionOnlyOnGamepad)
		{
			bShouldShow = (LastInputType == ECommonInputType::Gamepad);
		}

		if (bShouldShow)
			InputActionWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		else
			InputActionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	FText TextToDisplay = ButtonText;
	if (TextToDisplay.IsEmpty() && InputActionWidget)
		TextToDisplay = InputActionWidget->GetDisplayText();
	
	if (ButtonLabel)
		ButtonLabel->SetText(TextToDisplay);

	if (Icon && ButtonIcon)
		Icon->SetBrushFromTexture(ButtonIcon);
	
	switch (ContentMode)
	{
		case EButtonContentMode::TextOnly:
			if (ButtonLabel) 
				ButtonLabel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (Icon) 
				Icon->SetVisibility(ESlateVisibility::Collapsed);
			break;

		case EButtonContentMode::IconOnly:
			if (Icon) 
				Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (ButtonLabel) 
				ButtonLabel->SetVisibility(ESlateVisibility::Collapsed);
			break;

		case EButtonContentMode::TextAndIcon:
			if (ButtonLabel) 
				ButtonLabel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (Icon) 
				Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
	}
}

void UButtonBase::SetButtonContent(const FText& InText, UTexture2D* InIcon)
{
	ButtonText = InText;
	ButtonIcon = InIcon;
    
	RefreshButtonContent();
}

void UButtonBase::SetIsPersistentBinding(bool bInIsPersistentBinding)
{
	if (FBoolProperty* Prop = CastField<FBoolProperty>(UCommonButtonBase::StaticClass()->FindPropertyByName(TEXT("bIsPersistentBinding"))))
	{
		Prop->SetPropertyValue_InContainer(this, bInIsPersistentBinding);
	}
}

void UButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	if (ButtonLabel)
		ButtonLabel->SetStyle(GetCurrentTextStyleClass());

	if (Icon && GetCurrentTextStyleClass())
	{
		const UCommonTextStyle* TextStyle = GetCurrentTextStyleClass()->GetDefaultObject<UCommonTextStyle>();

		if (TextStyle)
			Icon->SetBrushTintColor(TextStyle->Color);
	}

	UpdateFocusIndicator();
}

FReply UButtonBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
	UpdateFocusIndicator();
	return Reply;
}

void UButtonBase::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	UpdateFocusIndicator();
}

void UButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Forward);

	if (bFocusOnHover && LastInputType == ECommonInputType::MouseAndKeyboard)
	{
		SetFocus();
	}

	UpdateFocusIndicator();
}

void UButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Reverse);
	UpdateFocusIndicator();
}

void UButtonBase::UpdateFocusIndicator()
{
	if (FocusIndicator)
	{
		const bool bShouldShow = HasAnyUserFocus() || (bFocusOnHover && IsHovered()) || GetSelected();
		FocusIndicator->SetVisibility(bShouldShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}
}


void UButtonBase::NativeOnReleased()
{
	Super::NativeOnReleased();

	// Ensure hover state is visually correct if we release while still hovering
	if (IsHovered())
		PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Forward);
	else
		PlayReversibleAnimation(HoveredAnimation, EUMGSequencePlayMode::Reverse);
}

void UButtonBase::NativeOnClicked()
{
	Super::NativeOnClicked();

	PlayPressedAnimation();
}

void UButtonBase::NativeConstruct()
{
	Super::NativeConstruct();

	CreationTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	RefreshButtonContent();
	UpdateFocusIndicator();
}

void UButtonBase::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);

	UpdateFocusIndicator();

	// Only play animation on selection if initialization/construction phase is complete (more than 0.05 seconds since creation)
	if (GetWorld() && (GetWorld()->GetTimeSeconds() - CreationTimeSeconds) > 0.05f)
	{
		PlayPressedAnimation();
	}
}

void UButtonBase::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);

	UpdateFocusIndicator();
}

void UButtonBase::PlayPressedAnimation()
{
	if (PressedAnimation)
	{
		if (UWorld* World = GetWorld())
		{
			uint64 CurrentFrame = GFrameCounter;
			if (LastPressedAnimationPlayFrame == CurrentFrame)
			{
				return; // Already played this frame
			}
			LastPressedAnimationPlayFrame = CurrentFrame;

			StopAnimation(PressedAnimation);
			PlayAnimation(PressedAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);

			FTimerHandle TempHandle;
			World->GetTimerManager().SetTimer(TempHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (PressedAnimation)
				{
					PlayReversibleAnimation(PressedAnimation, EUMGSequencePlayMode::Reverse);
				}
			}), 0.1f, false);
		}
	}
}

void UButtonBase::HandleActionClick()
{
	if (!ButtonActionTag.IsValid()) 
		return;
	
	// Route the action to the Activatable Widget's parent
	if (UActivatableWidgetBase* Parent = GetTypedOuter<UActivatableWidgetBase>())
		Parent->RouteAction(ButtonActionTag, this);
}

void UButtonBase::OnInputMethodChanged(const ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);
	LastInputType = CurrentInputType;

	if (bDisableFocusOnGamepad)
	{
		const bool bIsMouseAndKeyboard = CurrentInputType == ECommonInputType::MouseAndKeyboard;
		SetIsFocusable(bIsMouseAndKeyboard);
	}
	else
	{
		SetIsFocusable(true);
	}
	
	// Refresh visual style when switching from Gamepad to Keyboard
	UpdateButtonStyle();
	RefreshButtonContent();
	UpdateFocusIndicator();
}

void UButtonBase::PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode)
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
#pragma endregion