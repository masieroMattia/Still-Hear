#include "DevTools/DevLevelSelectWidget.h"

#include "DevTools/DevToolsSubsystem.h"
#include "UI/Elements/SaveSlotButtonBase.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"

void UDevLevelSelectWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Populate();
}

void UDevLevelSelectWidget::Populate()
{
	FirstButton = nullptr;

	if (!ChapterList || !ChapterButtonClass)
		return;

	ChapterList->ClearChildren();

	UGameInstance* GI = GetGameInstance();
	UDevToolsSubsystem* Dev = GI ? GI->GetSubsystem<UDevToolsSubsystem>() : nullptr;
	if (!Dev)
		return;

	const TArray<FDevChapterEntry> Chapters = Dev->GetChapters();
	for (const FDevChapterEntry& Entry : Chapters)
	{
		USaveSlotButtonBase* Button = CreateWidget<USaveSlotButtonBase>(this, ChapterButtonClass);
		if (!Button)
			continue;

		// Single click just selects/focuses (handled by USaveSlotButtonBase -> shows the focus
		// indicator); double click confirms the skip.
		Button->OnDoubleClicked().AddUObject(this, &UDevLevelSelectWidget::HandleChapterClicked, Entry.RowName);

		// Add first: AddChild triggers the button's NativeConstruct (which runs RefreshSlotInfo and
		// would read an empty disk slot). Apply our chapter display AFTER so it wins.
		ChapterList->AddChild(Button);
		Button->ApplyDisplay(Entry.DisplayName, Entry.Thumbnail, Entry.DisplayName);

		if (!FirstButton)
			FirstButton = Button;
	}
}

void UDevLevelSelectWidget::HandleChapterClicked(FName RowName)
{
	UGameInstance* GI = GetGameInstance();
	if (UDevToolsSubsystem* Dev = GI ? GI->GetSubsystem<UDevToolsSubsystem>() : nullptr)
		Dev->SkipToChapter(RowName);
}

UWidget* UDevLevelSelectWidget::NativeGetDesiredFocusTarget() const
{
	if (FirstButton)
		return FirstButton;

	return Super::NativeGetDesiredFocusTarget();
}
