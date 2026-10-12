#include "UI/Elements/TabWidgetBase.h"

#include "UI/Elements/ButtonBase.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonActivatableWidget.h"
#include "CommonTabListWidgetBase.h"
#include "UI/Widgets/ActivatableWidgetBase.h"

#pragma region METHODS

void UTabWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TabButtonsContainer || !ContentSwitcher || !TabButtonClass)
		return;

	TabButtonsContainer->SetLinkedSwitcher(ContentSwitcher);

	ContentSwitcher->OnTransitioningChanged.AddUObject(this, &UTabWidgetBase::HandleSwitcherTransitionFinished);
	
	const int32 NumTabs = ContentSwitcher->GetChildrenCount();
	for (int32 i = 0; i < NumTabs; ++i)
	{
		const FName TabId = FName(*FString::Printf(TEXT("Tab_%d"), i));
        
		// Register the tab
		TabButtonsContainer->RegisterTab(TabId, TabButtonClass, ContentSwitcher->GetWidgetAtIndex(i));

		// Retrieve the dynamically generated button
		if (UCommonButtonBase* SpawnedButton = TabButtonsContainer->GetTabButtonBaseByID(TabId))
		{
			if (UButtonBase* CustomButton = Cast<UButtonBase>(SpawnedButton))
			{
				// Fallback text if the array is not configured properly in the editor
				FText NameToSet = FText::FromString(TabId.ToString());
				UTexture2D* IconToSet = nullptr;

				if (TabsDisplayData.IsValidIndex(i))
				{
					NameToSet = TabsDisplayData[i].TabName;
					IconToSet = TabsDisplayData[i].TabIcon;
				}

				// Inject the data into the button
				CustomButton->SetButtonContent(NameToSet, IconToSet);
			}
		}
	}
	// Initialize Prev/Next Tab buttons if they exist
	if (PrevTabButton)
	{
		PrevTabButton->OnClicked().RemoveAll(this);
		PrevTabButton->OnClicked().AddUObject(this, &UTabWidgetBase::HandlePrevTabClicked);
		if (!PrevTabInputAction.IsNull())
		{
			PrevTabButton->SetIsPersistentBinding(true);
			PrevTabButton->SetTriggeringInputAction(PrevTabInputAction);
		}
	}

	if (NextTabButton)
	{
		NextTabButton->OnClicked().RemoveAll(this);
		NextTabButton->OnClicked().AddUObject(this, &UTabWidgetBase::HandleNextTabClicked);
		if (!NextTabInputAction.IsNull())
		{
			NextTabButton->SetIsPersistentBinding(true);
			NextTabButton->SetTriggeringInputAction(NextTabInputAction);
		}
	}
}


UWidget* UTabWidgetBase::NativeGetDesiredFocusTarget() const
{
	if (!ContentSwitcher)
		return Super::NativeGetDesiredFocusTarget();

	UWidget* ActivePage = ContentSwitcher->GetActiveWidget();
	if (!ActivePage)
		return Super::NativeGetDesiredFocusTarget();

	// Traverse one level deeper so CommonUI lands on the slot button, not the page container
	if (UActivatableWidgetBase* Activatable = Cast<UActivatableWidgetBase>(ActivePage))
	{
		if (UWidget* DeepTarget = Activatable->GetPreferredFocusTarget())
			return DeepTarget;
	}

	return ActivePage;
}

void UTabWidgetBase::HandleSwitcherTransitionFinished(const bool bIsTransitioning)
{
	if (!bIsTransitioning && ContentSwitcher)
	{
		if (UWidget* ActivePage = ContentSwitcher->GetActiveWidget())
		{
			UWidget* FocusTarget = ActivePage;
			if (UActivatableWidgetBase* Activatable = Cast<UActivatableWidgetBase>(ActivePage))
			{
				if (UWidget* DeepTarget = Activatable->GetPreferredFocusTarget())
					FocusTarget = DeepTarget;
			}
			FocusTarget->SetFocus();
		}
	}
}

void UTabWidgetBase::HandlePrevTabClicked()
{
	if (!TabButtonsContainer)
		return;

	const int32 TabCount = TabButtonsContainer->GetTabCount();
	if (TabCount <= 1)
		return;

	const FName ActiveTab = TabButtonsContainer->GetActiveTab();
	int32 ActiveIndex = INDEX_NONE;

	for (int32 i = 0; i < TabCount; ++i)
	{
		if (TabButtonsContainer->GetTabIdAtIndex(i) == ActiveTab)
		{
			ActiveIndex = i;
			break;
		}
	}

	if (ActiveIndex != INDEX_NONE)
	{
		const int32 PrevIndex = (ActiveIndex - 1 + TabCount) % TabCount;
		const FName PrevTabId = TabButtonsContainer->GetTabIdAtIndex(PrevIndex);
		TabButtonsContainer->SelectTabByID(PrevTabId);
	}
}

void UTabWidgetBase::HandleNextTabClicked()
{
	if (!TabButtonsContainer)
		return;

	const int32 TabCount = TabButtonsContainer->GetTabCount();
	if (TabCount <= 1)
		return;

	const FName ActiveTab = TabButtonsContainer->GetActiveTab();
	int32 ActiveIndex = INDEX_NONE;

	for (int32 i = 0; i < TabCount; ++i)
	{
		if (TabButtonsContainer->GetTabIdAtIndex(i) == ActiveTab)
		{
			ActiveIndex = i;
			break;
		}
	}

	if (ActiveIndex != INDEX_NONE)
	{
		const int32 NextIndex = (ActiveIndex + 1) % TabCount;
		const FName NextTabId = TabButtonsContainer->GetTabIdAtIndex(NextIndex);
		TabButtonsContainer->SelectTabByID(NextTabId);
	}
}
#pragma endregion