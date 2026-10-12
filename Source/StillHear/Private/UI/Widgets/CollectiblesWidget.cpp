#include "UI/Widgets/CollectiblesWidget.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "CommonInputSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "UI/Elements/ButtonBase.h"
#include "Input/CommonUIInputTypes.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Components/ScrollBoxSlot.h"
#include "Data/DataTables/CollectibleData.h"
#include "UI/Elements/CollectibleSlotButton.h"

static bool bIsProgrammaticScroll = false;
static float ProgrammaticScrollStartTime = 0.f;

void UCollectiblesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResetButton)
	{
		ResetButton->OnClicked().RemoveAll(this);
		ResetButton->OnClicked().AddUObject(this, &UCollectiblesWidget::HandleResetClicked);
		if (!ResetInputAction.IsNull())
		{
			ResetButton->SetIsPersistentBinding(true);
			ResetButton->SetTriggeringInputAction(ResetInputAction);
		}
	}

	InitializeCollectiblesGrid();

	// Force activation on construct to guarantee CommonUI routes gamepad inputs to this widget's buttons
	if (!IsActivated())
		ActivateWidget();

	bNeedsFocusRestore = true;
}

void UCollectiblesWidget::NativeOnActivated()
{
	InitializeCollectiblesGrid();
	Super::NativeOnActivated();
	bNeedsFocusRestore = true;
}

UWidget* UCollectiblesWidget::GetPreferredFocusTarget_Implementation() const
{
	return SelectedSlotButton ? SelectedSlotButton : Super::GetPreferredFocusTarget_Implementation();
}

void UCollectiblesWidget::RestoreFocus()
{
	if (!SelectedSlotButton)
		return;

	// Drive focus through the widget's own selection system so bIsWheelSelected is set
	// and the button is properly registered as the active item for gamepad navigation
	HandleSlotButtonHovered(SelectedSlotButton);
	SelectedSlotButton->SetFocus();
}

void UCollectiblesWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

void UCollectiblesWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	if (!ResetInputAction.IsNull() && !ResetBindingHandle.IsValid())
	{
		FBindUIActionArgs BindArgs(ResetInputAction, FSimpleDelegate::CreateUObject(this, &UCollectiblesWidget::HandleResetClicked));
		BindArgs.bDisplayInActionBar = false;
		ResetBindingHandle = RegisterUIActionBinding(BindArgs);
	}
}

void UCollectiblesWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	if (ResetBindingHandle.IsValid())
	{
		ResetBindingHandle.Unregister();
	}
}

void UCollectiblesWidget::InitializeCollectiblesGrid()
{
	if (CollectiblesGrid)
	{
		CollectiblesGrid->ClearChildren();
		CollectiblesGrid->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
	}

	TopSpacer = nullptr;

	TArray<FCollectibleItemInfo> CollectibleItems;
	TArray<UCollectibleSlotButton*> ButtonsArray;

	if (CollectiblesDataTable && CollectibleSlotButtonClass)
	{
		const USaveSubsystem* SaveSubsystem = nullptr;
		if (const UWorld* World = GetWorld())
		{
			if (const UGameInstance* GI = World->GetGameInstance())
			{
				SaveSubsystem = GI->GetSubsystem<USaveSubsystem>();
			}
		}

		TArray<FName> RowNames = CollectiblesDataTable->GetRowNames();

		for (const FName& RowName : RowNames)
		{
			if (const FCollectibleData* RowData = CollectiblesDataTable->FindRow<FCollectibleData>(RowName, TEXT("CollectiblesMenu")))
			{
				FCollectibleItemInfo Item;
				Item.RowName = RowName;
				Item.DescriptionText = RowData->CollectibleText;
				Item.Image = RowData->CollectibleImage;
				Item.Material = RowData->CollectibleMaterial;
				Item.bIsCollected = SaveSubsystem ? SaveSubsystem->IsCollectibleCollected(RowName) : false;

				CollectibleItems.Add(Item);

				UCollectibleSlotButton* NewButton = CreateWidget<UCollectibleSlotButton>(this, CollectibleSlotButtonClass);
				if (NewButton)
				{
					NewButton->InitializeSlot(Item, LockedItemText, LockedItemImage, LockedItemMaterial);
					
					NewButton->OnSlotButtonHovered.RemoveAll(this);
					NewButton->OnSlotButtonHovered.AddDynamic(this, &UCollectiblesWidget::HandleSlotButtonHovered);

					NewButton->OnSlotButtonClicked.RemoveAll(this);
					NewButton->OnSlotButtonClicked.AddDynamic(this, &UCollectiblesWidget::HandleSlotButtonClicked);

					if (CollectiblesGrid)
					{
						CollectiblesGrid->AddChild(NewButton);
						ButtonsArray.Add(NewButton);
					}
				}
			}
		}

		// Establish explicit vertical navigation links via the runtime API so Slate actually picks them up
		const int32 NumButtons = ButtonsArray.Num();
		for (int32 i = 0; i < NumButtons; ++i)
		{
			UCollectibleSlotButton* CurrentBtn = ButtonsArray[i];
			if (!CurrentBtn)
				continue;

			if (i > 0)
				CurrentBtn->SetNavigationRuleExplicit(EUINavigation::Up, ButtonsArray[i - 1]);
			else
				CurrentBtn->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Escape);

			if (i < NumButtons - 1)
				CurrentBtn->SetNavigationRuleExplicit(EUINavigation::Down, ButtonsArray[i + 1]);
			else
				CurrentBtn->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Escape);

			CurrentBtn->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Escape);
			CurrentBtn->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Escape);
		}

		// Restore previous selection or default to the first button
		UCollectibleSlotButton* ButtonToSelect = nullptr;
		if (SelectedSlotButton)
		{
			const FName PrevRowName = SelectedSlotButton->GetItemInfo().RowName;
			for (UCollectibleSlotButton* Btn : ButtonsArray)
			{
				if (Btn && Btn->GetItemInfo().RowName == PrevRowName)
				{
					ButtonToSelect = Btn;
					break;
				}
			}
		}

		if (!ButtonToSelect && NumButtons > 0)
		{
			ButtonToSelect = ButtonsArray[0];
		}

		SelectedSlotButton = ButtonToSelect;

		RefreshAllFocusIndicators();

		if (SelectedSlotButton && CollectiblesGrid)
		{
			CollectiblesGrid->ScrollWidgetIntoView(SelectedSlotButton, true, EDescendantScrollDestination::Center);
		}
	}

	BottomSpacer = nullptr;

	if (CollectiblesGrid)
	{
		CollectiblesGrid->OnUserScrolled.RemoveAll(this);
		CollectiblesGrid->OnUserScrolled.AddDynamic(this, &UCollectiblesWidget::HandleUserScrolled);
	}

	BP_OnCollectiblesInitialized(CollectibleItems);
}

void UCollectiblesWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bNeedsFocusRestore)
	{
		bNeedsFocusRestore = false;
		RestoreFocus();
	}

	if (!CollectiblesGrid)
		return;

	// Rebuild only when the number of collected items changes — avoids GetRowNames() every tick
	// and prevents infinite rebuild loops if ExpectedButtonCount mismatches persistently.
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
		{
			const int32 SavedCount = SaveSubsystem->GetCollectedCollectibles().Num();
			int32 GridCollectedCount = 0;
			const int32 NumChildren = CollectiblesGrid->GetChildrenCount();
			for (int32 i = 0; i < NumChildren; ++i)
			{
				if (const UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
				{
					if (SlotBtn->GetItemInfo().bIsCollected)
						GridCollectedCount++;
				}
			}

			if (SavedCount != GridCollectedCount)
			{
				InitializeCollectiblesGrid();
				return;
			}
		}
	}

	const FGeometry ScrollBoxGeometry = CollectiblesGrid->GetCachedGeometry();
	const FVector2D ScrollBoxSize = ScrollBoxGeometry.GetLocalSize();
	if (ScrollBoxSize.X <= 0.f || ScrollBoxSize.Y <= 0.f)
		return;

	static float LastLogTime = 0.f;
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (CurrentTime - LastLogTime > 1.0f)
	{
		LastLogTime = CurrentTime;
		float CurrentFirstPadding = 0.f;
		if (CollectiblesGrid && CollectiblesGrid->GetChildrenCount() > 0)
		{
			if (UWidget* FirstChild = CollectiblesGrid->GetChildAt(0))
			{
				if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(FirstChild->Slot))
				{
					CurrentFirstPadding = ScrollSlot->GetPadding().Top;
				}
			}
		}
	}

	if (bIsProgrammaticScroll)
	{
		bool bClearFlag = false;
		if (UWorld* World = GetWorld())
		{
			if (World->GetTimeSeconds() - ProgrammaticScrollStartTime > 0.4f)
			{
				bClearFlag = true;
			}
		}

		if (!bClearFlag && SelectedSlotButton)
		{
			const FVector2D ScrollBoxCenter = ScrollBoxSize * 0.5f;
			const FGeometry ChildGeometry = SelectedSlotButton->GetCachedGeometry();
			const FVector2D ChildAbsolutePos = ChildGeometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
			const FVector2D ChildLocalPos = ScrollBoxGeometry.AbsoluteToLocal(ChildAbsolutePos);
			const float DistanceY = FMath::Abs(ChildLocalPos.Y - ScrollBoxCenter.Y);
			if (DistanceY < 5.f)
			{
				bClearFlag = true;
			}
		}

		if (bClearFlag)
		{
			bIsProgrammaticScroll = false;
		}
	}

	const FVector2D ScrollBoxCenter = ScrollBoxSize * 0.5f;

	const int32 NumChildren = CollectiblesGrid->GetChildrenCount();

	// Dynamic Padding sizing on slot boundaries based on actual button heights
	float ButtonHeight = 100.f; // default fallback
	UCollectibleSlotButton* FirstButton = nullptr;
	UCollectibleSlotButton* LastButton = nullptr;

	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UCollectibleSlotButton* ButtonChild = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
		{
			if (!FirstButton)
			{
				FirstButton = ButtonChild;
			}
			LastButton = ButtonChild;

			const FVector2D ButtonSize = ButtonChild->GetCachedGeometry().GetLocalSize();
			if (ButtonSize.Y > 0.f)
			{
				ButtonHeight = ButtonSize.Y;
			}
		}
	}

	const float TargetPadding = FMath::Max(0.f, (ScrollBoxSize.Y - ButtonHeight) * 0.5f);

	if (FirstButton)
	{
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(FirstButton->Slot))
		{
			FMargin CurrentPadding = ScrollSlot->GetPadding();
			if (!FMath::IsNearlyEqual(CurrentPadding.Top, TargetPadding))
			{
				CurrentPadding.Top = TargetPadding;
				ScrollSlot->SetPadding(CurrentPadding);
			}
		}
	}

	if (LastButton)
	{
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(LastButton->Slot))
		{
			FMargin CurrentPadding = ScrollSlot->GetPadding();
			if (!FMath::IsNearlyEqual(CurrentPadding.Bottom, TargetPadding))
			{
				CurrentPadding.Bottom = TargetPadding;
				ScrollSlot->SetPadding(CurrentPadding);
			}
		}
	}

	// Calculate maximum distance (distance from center to top/bottom edges of the scrollbox, scaled by EffectRadiusScale)
	const float MaxDistance = ScrollBoxSize.Y * 0.5f * FMath::Max(0.01f, EffectRadiusScale);

	// Scale, translate and fade child buttons based on their distance from center
	for (int32 i = 0; i < NumChildren; ++i)
	{
		UWidget* Child = CollectiblesGrid->GetChildAt(i);
		UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(Child);
		if (!SlotBtn)
			continue;

		const FGeometry ChildGeometry = SlotBtn->GetCachedGeometry();
		const FVector2D ChildAbsolutePos = ChildGeometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
		const FVector2D ChildLocalPos = ScrollBoxGeometry.AbsoluteToLocal(ChildAbsolutePos);

		// Distance along the vertical axis (Y) from the center of the scrollbox
		const float DistanceY = ChildLocalPos.Y - ScrollBoxCenter.Y;

		// Normalized distance (0 at the center, 1 at the top/bottom edges)
		const float NormalizedDistance = FMath::Clamp(FMath::Abs(DistanceY) / MaxDistance, 0.0f, 1.0f);

		// Apply exponents to control shape/transition style
		const float CurveFactor = FMath::Pow(NormalizedDistance, FMath::Max(0.01f, CurveExponent));
		const float ScaleFactor = FMath::Pow(NormalizedDistance, FMath::Max(0.01f, ScaleExponent));
		const float OpacityFactor = FMath::Pow(NormalizedDistance, FMath::Max(0.01f, OpacityExponent));

		// Dynamic Scaling (Center is larger, edges are smaller)
		const float ScaleVal = FMath::Lerp(CenterItemScale, EdgeItemScale, ScaleFactor);
		SlotBtn->SetRenderScale(FVector2D(ScaleVal, ScaleVal));

		//  Wheel/Curved path offset (Translate on the X axis to create a circular path)
		const float CurveOffsetX = FMath::Lerp(MaxCurveOffset, 0.0f, CurveFactor);
		SlotBtn->SetRenderTranslation(FVector2D(CurveOffsetX, 0.0f));

		// Opacity blending (Center is fully visible, edges fade out)
		const float OpacityVal = FMath::Lerp(1.0f, EdgeItemOpacity, OpacityFactor);
		SlotBtn->SetRenderOpacity(OpacityVal);
	}
}

UWidget* UCollectiblesWidget::NativeGetDesiredFocusTarget() const
{
	if (SelectedSlotButton)
	{
		return SelectedSlotButton;
	}

	if (CollectiblesGrid)
	{
		const int32 NumChildren = CollectiblesGrid->GetChildrenCount();
		for (int32 i = 0; i < NumChildren; ++i)
		{
			if (UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
			{
				return SlotBtn;
			}
		}
	}
	return Super::NativeGetDesiredFocusTarget();
}

FReply UCollectiblesWidget::NativeOnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!CollectiblesGrid)
		return Super::NativeOnMouseWheel(MyGeometry, MouseEvent);

	const float WheelDelta = MouseEvent.GetWheelDelta();
	if (FMath::IsNearlyZero(WheelDelta))
		return Super::NativeOnMouseWheel(MyGeometry, MouseEvent);

	const int32 NumChildren = CollectiblesGrid->GetChildrenCount();
	const UCollectibleSlotButton* CurrentFocused = nullptr;
	int32 FocusedIndex = INDEX_NONE;

	// Find the currently focused slot button
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (const UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
		{
			if (SlotBtn->HasAnyUserFocus())
			{
				CurrentFocused = SlotBtn;
				FocusedIndex = i;
				break;
			}
		}
	}

	int32 TargetIndex = INDEX_NONE;
	if (FocusedIndex == INDEX_NONE)
	{
		// Fallback: If nothing currently has keyboard focus, find the index of our active SelectedSlotButton
		if (SelectedSlotButton)
		{
			for (int32 i = 0; i < NumChildren; ++i)
			{
				if (CollectiblesGrid->GetChildAt(i) == SelectedSlotButton)
				{
					FocusedIndex = i;
					break;
				}
			}
		}
	}

	if (FocusedIndex == INDEX_NONE)
	{
		// If still nothing is focused/selected, find and focus the first available slot button
		for (int32 i = 0; i < NumChildren; ++i)
		{
			if (Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
			{
				TargetIndex = i;
				break;
			}
		}
	}
	else
	{
		// Determine discrete navigation direction based on scroll delta direction
		if (WheelDelta > 0.f)
		{
			// Scroll UP -> go to the previous focusable slot button
			for (int32 i = FocusedIndex - 1; i >= 0; --i)
			{
				if (Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
				{
					TargetIndex = i;
					break;
				}
			}
		}
		else
		{
			// Scroll DOWN -> go to the next focusable slot button
			for (int32 i = FocusedIndex + 1; i < NumChildren; ++i)
			{
				if (Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
				{
					TargetIndex = i;
					break;
				}
			}
		}
	}

	if (TargetIndex != INDEX_NONE)
	{
		if (UCollectibleSlotButton* TargetButton = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(TargetIndex)))
		{
			SelectedSlotButton = TargetButton;
			RefreshAllFocusIndicators();

			// Scroll into view
			bIsProgrammaticScroll = true;
			ProgrammaticScrollStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
			CollectiblesGrid->ScrollWidgetIntoView(TargetButton, true, EDescendantScrollDestination::Center);

			// Direct focus
			TargetButton->SetFocus();

			// Update details panel
			HandleSlotButtonHovered(TargetButton);

			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseWheel(MyGeometry, MouseEvent);
}

void UCollectiblesWidget::HandleSlotButtonHovered(UCollectibleSlotButton* SlotButton)
{
	if (!SlotButton)
		return;

	SelectedSlotButton = SlotButton;
	RefreshAllFocusIndicators();

	const FCollectibleItemInfo& Info = SlotButton->GetItemInfo();

	if (Info.bIsCollected)
	{
		if (DetailNameText)
		{
			DetailNameText->SetText(CollectibleRowNameToDisplayText(Info.RowName));
		}
		if (DetailDescriptionText)
		{
			DetailDescriptionText->SetText(Info.DescriptionText);
		}
		if (DetailImage)
		{
			DetailImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (!Info.Material.IsNull())
			{
				DetailImage->SetBrushFromMaterial(Info.Material.LoadSynchronous());
			}
			else if (!Info.Image.IsNull())
			{
				DetailImage->SetBrushFromTexture(Info.Image.LoadSynchronous());
			}
			else
			{
				DetailImage->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	else
	{
		if (DetailNameText)
		{
			DetailNameText->SetText(LockedItemText);
		}
		if (DetailDescriptionText)
		{
			DetailDescriptionText->SetText(LockedItemText);
		}
		if (DetailImage)
		{
			DetailImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (!LockedItemMaterial.IsNull())
			{
				DetailImage->SetBrushFromMaterial(LockedItemMaterial.LoadSynchronous());
			}
			else if (!LockedItemImage.IsNull())
			{
				DetailImage->SetBrushFromTexture(LockedItemImage.LoadSynchronous());
			}
			else
			{
				DetailImage->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// Auto-scroll the focused item to the center of the viewport (ignoring mouse hover loops)
	if (CollectiblesGrid)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const UCommonInputSubsystem* InputSubsystem = LP ? LP->GetSubsystem<UCommonInputSubsystem>() : nullptr;
		bool bShouldScroll = false;
		if (InputSubsystem)
		{
			if (InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
			{
				bShouldScroll = true;
			}
			else if (SlotButton->HasAnyUserFocus() && !SlotButton->IsHovered())
			{
				// Keyboard arrow navigation
				bShouldScroll = true;
			}
		}
		else
		{
			bShouldScroll = SlotButton->HasAnyUserFocus() && !SlotButton->IsHovered();
		}

		if (bShouldScroll && !bDisableAutoCentering)
		{
			bIsProgrammaticScroll = true;
			ProgrammaticScrollStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
			CollectiblesGrid->ScrollWidgetIntoView(SlotButton, true, EDescendantScrollDestination::Center);
		}
	}
}

void UCollectiblesWidget::HandleSlotButtonClicked(UCollectibleSlotButton* SlotButton)
{
	if (CollectiblesGrid && SlotButton)
	{
		SelectedSlotButton = SlotButton;
		RefreshAllFocusIndicators();

		// Centering
		bIsProgrammaticScroll = true;
		ProgrammaticScrollStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		CollectiblesGrid->ScrollWidgetIntoView(SlotButton, true, EDescendantScrollDestination::Center);

		// Focus
		SlotButton->SetFocus();

		// Update details panel
		HandleSlotButtonHovered(SlotButton);
	}
}

void UCollectiblesWidget::HandleUserScrolled(float CurrentOffset)
{
	if (!CollectiblesGrid)
		return;

	if (bIsProgrammaticScroll)
		return;

	// Temporarily disable auto-centering to prevent snaps during manual scrolling
	bDisableAutoCentering = true;

	UCollectibleSlotButton* ClosestButton = FindClosestButtonToCenter();
	if (ClosestButton && !ClosestButton->HasAnyUserFocus())
	{
		ClosestButton->SetFocus();
	}

	bDisableAutoCentering = false;
}

void UCollectiblesWidget::RefreshAllFocusIndicators()
{
	if (!CollectiblesGrid) return;
	const int32 NumChildren = CollectiblesGrid->GetChildrenCount();
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
			SlotBtn->SetWheelSelected(SlotBtn == SelectedSlotButton);
	}
}

UCollectibleSlotButton* UCollectiblesWidget::FindClosestButtonToCenter() const
{
	if (!CollectiblesGrid)
		return nullptr;

	const FGeometry ScrollBoxGeometry = CollectiblesGrid->GetCachedGeometry();
	const FVector2D ScrollBoxSize = ScrollBoxGeometry.GetLocalSize();
	if (ScrollBoxSize.Y <= 0.f)
		return nullptr;

	const FVector2D ScrollBoxCenter = ScrollBoxSize * 0.5f;

	UCollectibleSlotButton* ClosestButton = nullptr;
	float MinDistanceY = FLT_MAX;

	const int32 NumChildren = CollectiblesGrid->GetChildrenCount();
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (UCollectibleSlotButton* SlotBtn = Cast<UCollectibleSlotButton>(CollectiblesGrid->GetChildAt(i)))
		{
			const FGeometry ChildGeometry = SlotBtn->GetCachedGeometry();
			const FVector2D ChildAbsolutePos = ChildGeometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
			const FVector2D ChildLocalPos = ScrollBoxGeometry.AbsoluteToLocal(ChildAbsolutePos);
			const float DistanceY = FMath::Abs(ChildLocalPos.Y - ScrollBoxCenter.Y);
			
			if (DistanceY < MinDistanceY)
			{
				MinDistanceY = DistanceY;
				ClosestButton = SlotBtn;
			}
		}
	}

	return ClosestButton;
}

void UCollectiblesWidget::HandleResetClicked()
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
		{
			SaveSub->ResetCollectibles();
			InitializeCollectiblesGrid();
		}
	}
}