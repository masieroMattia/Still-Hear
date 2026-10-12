#include "UI/Widgets/TutorialPopupWidget.h"

#include "InputAction.h"
#include "CommonTextBlock.h"
#include "CommonActionWidget.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "EnhancedInputSubsystems.h"
#include "UI/Widgets/InputActionWidget.h"
#include "GameFramework/PlayerController.h"

#pragma region UFUNCTIONS
void UTutorialPopupWidget::InitializeTutorial(const FText& InMessage, const TArray<UInputAction*>& InputActions, float InDuration)
{
	// Initialize the base popup properties
	InitializePopup(InMessage, InDuration);

	if (!Container || !TextBlockClass || !InputActionWidgetClass)
	{
		return;
	}

	// Clear existing widgets if any
	Container->ClearChildren();
	TrackedActionWidgets.Empty();

	const FString SourceString = InMessage.ToString();
	int32 CurrentIndex = 0;

	// Parse the string for {x}
	while (CurrentIndex < SourceString.Len())
	{
		const int32 OpenBraceIndex = SourceString.Find(TEXT("{"), ESearchCase::IgnoreCase, ESearchDir::FromStart, CurrentIndex);
		
		if (OpenBraceIndex == INDEX_NONE)
		{
			// No more braces, add the remaining text
			const FString Remainder = SourceString.Mid(CurrentIndex);
			AddTextSegment(Remainder);
			break;
		}

		// Add the text before the brace
		if (OpenBraceIndex > CurrentIndex)
		{
			FString BeforeBrace = SourceString.Mid(CurrentIndex, OpenBraceIndex - CurrentIndex);
			AddTextSegment(BeforeBrace);
		}

		// Find the closing brace
		const int32 CloseBraceIdx = SourceString.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, OpenBraceIndex);
		if (CloseBraceIdx != INDEX_NONE)
		{
			// Extract the number inside the braces
			FString NumberString = SourceString.Mid(OpenBraceIndex + 1, CloseBraceIdx - OpenBraceIndex - 1);
			if (NumberString.IsNumeric())
			{
				const int32 ActionIndex = FCString::Atoi(*NumberString); // Convert to integer
				
				// Validate the index and spawn the action widget
				if (InputActions.IsValidIndex(ActionIndex) && InputActions[ActionIndex] != nullptr)
				{
					if (UInputActionWidget* ActionWidget = CreateWidget<UInputActionWidget>(this, InputActionWidgetClass))
					{
						// Automatically assign the InputAction
						ActionWidget->SetupInputAction(InputActions[ActionIndex]);
						
						// Call the BP event for backward compatibility
						OnInputActionWidgetSpawned(ActionWidget, InputActions[ActionIndex]);
						
						// Add to tracked widgets for Tick updating
						TrackedActionWidgets.Add({ActionWidget, InputActions[ActionIndex], false});
						
						if (UWrapBoxSlot* WrapSlot = Container->AddChildToWrapBox(ActionWidget))
						{
							WrapSlot->SetVerticalAlignment(VAlign_Center);
						}
					}
				}
			}
			CurrentIndex = CloseBraceIdx + 1;
		}
		else
		{
			// Missing closing brace, just add the rest of the string and break
			const FString Remainder = SourceString.Mid(OpenBraceIndex);
			AddTextSegment(Remainder);
			break;
		}
	}
}
#pragma endregion

#pragma region METHODS
void UTutorialPopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Poll input states for dynamic widgets
	ProcessActionWidgetInput();
}

void UTutorialPopupWidget::AddTextSegment(const FString& TextSegment)
{
	if (TextSegment.IsEmpty() || !TextBlockClass || !Container)
	{
		return;
	}

	if (UCommonTextBlock* TextBlock = NewObject<UCommonTextBlock>(this, TextBlockClass))
	{
		TextBlock->SetText(FText::FromString(TextSegment));
		if (UWrapBoxSlot* WrapSlot = Container->AddChildToWrapBox(TextBlock))
		{
			WrapSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

void UTutorialPopupWidget::ProcessActionWidgetInput()
{
	if (TrackedActionWidgets.IsEmpty())
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem)
		return;

	// Update each tracked widget individually
	for (FActionWidgetState& State : TrackedActionWidgets)
	{
		UpdateActionWidgetState(State, PC, Subsystem);
	}
}

void UTutorialPopupWidget::UpdateActionWidgetState(FActionWidgetState& State, APlayerController* PC, UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	if (!State.Widget || !State.Action || !PC || !Subsystem)
		return;

	// Check if any of the physical keys mapped to this action are currently pressed
	TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(State.Action);
	bool bIsCurrentlyPressed = false;

	for (const FKey& Key : Keys)
	{
		if (PC->IsInputKeyDown(Key))
		{
			bIsCurrentlyPressed = true;
			break;
		}
	}

	// Handle Press state transition
	if (bIsCurrentlyPressed && !State.bWasPressed)
	{
		State.bWasPressed = true;
		
		if (UInputActionWidget* ActionWidget = Cast<UInputActionWidget>(State.Widget))
		{
			ActionWidget->PressAction();
		}
		
		OnInputActionWidgetPressed(State.Widget, State.Action);
	}
	
	// Handle Release state transition
	else if (!bIsCurrentlyPressed && State.bWasPressed)
	{
		State.bWasPressed = false;
		
		if (UInputActionWidget* ActionWidget = Cast<UInputActionWidget>(State.Widget))
		{
			ActionWidget->ReleaseAction();
		}
		
		OnInputActionWidgetReleased(State.Widget, State.Action);
	}
}
#pragma endregion
		
#pragma region INTERFACE METHODS
void UTutorialPopupWidget::InitializeWidget_Implementation(const FText& Text, float Duration, const TArray<UInputAction*>& InputActions)
{
	InitializeTutorial(Text, InputActions, Duration);
}
#pragma endregion