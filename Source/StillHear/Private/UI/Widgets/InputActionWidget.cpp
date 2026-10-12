#include "UI/Widgets/InputActionWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "CommonActionWidget.h"
#include "Engine/LocalPlayer.h"
#include "Components/SizeBox.h"
#include "CommonInputSubsystem.h"
#include "Animation/WidgetAnimation.h"

#pragma region UPROPERTIES
void UInputActionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	bIsPressed = false;

	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UCommonInputSubsystem* InputSubsystem = LP->GetSubsystem<UCommonInputSubsystem>())
			{
				InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &UInputActionWidget::OnInputMethodChanged);
			}
		}
	}

	// Start normal idle animation
	if (NormalIdleAnim)
	{
		// 0 means loop infinitely
		PlayAnimation(NormalIdleAnim, 0.0f, 0); 
	}
}

void UInputActionWidget::PressAction()
{
	if (bIsPressed)
		return;

	bIsPressed = true;

	// Stop any looping idle animations
	if (NormalIdleAnim) 
		StopAnimation(NormalIdleAnim);
	
	if (PressedIdleAnim) 
		StopAnimation(PressedIdleAnim);

	if (PressAnim)
	{
		float StartTime = 0.0f;
		
		// If it's already playing (e.g., from a quick release), resume from its current time
		if (IsAnimationPlaying(PressAnim))
		{
			StartTime = GetAnimationCurrentTime(PressAnim);
		}
		
		PlayAnimation(PressAnim, StartTime, 1, EUMGSequencePlayMode::Forward, 1.0f);
	}
	else if (PressedIdleAnim)
	{
		// Fallback if no press transition exists
		PlayAnimation(PressedIdleAnim, 0.0f, 0);
	}
}

void UInputActionWidget::ReleaseAction()
{
	if (!bIsPressed)
		return;

	bIsPressed = false;

	// Stop any looping idle animations
	if (NormalIdleAnim) 
		StopAnimation(NormalIdleAnim);
	
	if (PressedIdleAnim) 
		StopAnimation(PressedIdleAnim);

	if (PressAnim)
	{
		if (IsAnimationPlaying(PressAnim))
		{
			// If the animation is currently playing reverse it from its current position
			ReverseAnimation(PressAnim);
		}
		else
		{
			// If the animation has already finished start playing it in reverse from the end
			PlayAnimation(PressAnim, 0.0f, 1, EUMGSequencePlayMode::Reverse);
		}
	}
	else if (NormalIdleAnim)
	{
		// Fallback if no press transition exists
		PlayAnimation(NormalIdleAnim, 0.0f, 0);
	}
}
#pragma endregion
	
#pragma region METHODS
void UInputActionWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	// When the press transition finishes, start the appropriate idle loop
	if (Animation == PressAnim)
	{
		if (bIsPressed)
		{
			if (PressedIdleAnim)
			{
				PlayAnimation(PressedIdleAnim, 0.0f, 0);
			}
		}
		else
		{
			if (NormalIdleAnim)
			{
				PlayAnimation(NormalIdleAnim, 0.0f, 0);
			}
		}
	}
}

void UInputActionWidget::SetupInputAction_Implementation(UInputAction* InputAction)
{
	if (!InputAction) 
		return;

	BoundAction = InputAction;

	if (CustomActionIcons.Contains(InputAction) && CustomIcon)
	{
		const FCustomIconData& IconData = CustomActionIcons[InputAction];
		CustomIcon->SetBrushFromTexture(IconData.IconTexture);
	}

	// Trigger initial state evaluation
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (const UCommonInputSubsystem* InputSubsystem = LP->GetSubsystem<UCommonInputSubsystem>())
			{
				OnInputMethodChanged(InputSubsystem->GetCurrentInputType());
				return;
			}
		}
	}
	
	// Default to Keyboard if subsystem is unavailable
	OnInputMethodChanged(ECommonInputType::MouseAndKeyboard);
}

void UInputActionWidget::OnInputMethodChanged(const ECommonInputType CurrentInputType)
{
	if (!BoundAction) 
		return;

	// Cache the values set in the editor the first time this function is called
	if (IconSizeBox && !bHasCachedSizeBox)
	{
		bHasCachedSizeBox = true;
		bHasDefaultWidthOverride = IconSizeBox->IsWidthOverride();
		DefaultWidthOverride = IconSizeBox->GetWidthOverride();
		
		bHasDefaultHeightOverride = IconSizeBox->IsHeightOverride();
		DefaultHeightOverride = IconSizeBox->GetHeightOverride();
	}

	// Only use the custom override if we are on Keyboard AND we have an override for this action
	if (CurrentInputType == ECommonInputType::MouseAndKeyboard && CustomActionIcons.Contains(BoundAction) && CustomIcon)
	{
		CustomIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (ActionIcon) 
		{
			// Unbind the action so CommonUI doesn't force its visibility back on
			ActionIcon->SetEnhancedInputAction(nullptr);
			ActionIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		// Apply custom dimensions explicitly (both width and height)
		if (IconSizeBox)
		{
			const auto& [IconTexture, IconSize] = CustomActionIcons[BoundAction];
			IconSizeBox->SetWidthOverride(IconSize.X);
			IconSizeBox->SetHeightOverride(IconSize.Y);
			CustomIcon->SetDesiredSizeOverride(IconSize);
		}
		else
		{
			const auto& [IconTexture, IconSize] = CustomActionIcons[BoundAction];
			CustomIcon->SetDesiredSizeOverride(IconSize);
		}
	}
	else
	{
		if (CustomIcon) 
			CustomIcon->SetVisibility(ESlateVisibility::Collapsed);
		
		if (ActionIcon) 
		{
			// Bind the real action so CommonUI shows the proper glyph
			ActionIcon->SetEnhancedInputAction(BoundAction);
			ActionIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		// Restore original overrides for the standard CommonUI icon
		if (IconSizeBox)
		{
			if (bHasDefaultWidthOverride) 
				IconSizeBox->SetWidthOverride(DefaultWidthOverride);
			else
				IconSizeBox->ClearWidthOverride();
				
			if (bHasDefaultHeightOverride)
				IconSizeBox->SetHeightOverride(DefaultHeightOverride);
			else
				IconSizeBox->ClearHeightOverride();
		}
	}
}
#pragma endregion