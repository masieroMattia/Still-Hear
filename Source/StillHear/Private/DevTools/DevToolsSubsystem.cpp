#include "DevTools/DevToolsSubsystem.h"

#include "DevTools/DevToolsSettings.h"
#include "DevTools/DevChapterRow.h"
#include "DevTools/DevChapterSaveData.h"
#include "SaveSystem/SaveSubsystem.h"
#include "UI/Subsystem/UISubsystem.h"
#include "CommonActivatableWidget.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

// ─────────────────────────────────────────────────────────────────────────────
// Slate input pre-processor: observes every key down globally (regardless of input
// mode or focus) and forwards it to the subsystem. It NEVER consumes input, so it
// cannot interfere with normal gameplay/menu navigation.
// ─────────────────────────────────────────────────────────────────────────────
class FDevKeySequenceProcessor : public IInputProcessor
{
public:
	explicit FDevKeySequenceProcessor(UDevToolsSubsystem* InOwner)
		: Owner(InOwner) {}

	virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication&, const FKeyEvent& InKeyEvent) override
	{
		if (UDevToolsSubsystem* Strong = Owner.Get())
			Strong->FeedKey(InKeyEvent.GetKey());
		return false; // never consume
	}

private:
	TWeakObjectPtr<UDevToolsSubsystem> Owner;
};

// ─────────────────────────────────────────────────────────────────────────────

void UDevToolsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FDevKeySequenceProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor.ToSharedRef());
	}
}

void UDevToolsSubsystem::Deinitialize()
{
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
	InputProcessor.Reset();

	Super::Deinitialize();
}

void UDevToolsSubsystem::FeedKey(const FKey& Key)
{
	const UDevToolsSettings* Settings = UDevToolsSettings::Get();
	if (!Settings || Settings->UnlockSequence.Num() == 0)
		return;

	const double Now = FPlatformTime::Seconds();
	if (MatchIndex > 0 && (Now - LastKeyTime) > Settings->SequenceResetSeconds)
		MatchIndex = 0;
	LastKeyTime = Now;

	if (Key == Settings->UnlockSequence[MatchIndex])
	{
		++MatchIndex;
		if (MatchIndex >= Settings->UnlockSequence.Num())
		{
			MatchIndex = 0;
			OnUnlockSequenceEntered();
		}
	}
	else
	{
		// A wrong key resets, but if it happens to be the first key of the sequence, count it.
		MatchIndex = (Key == Settings->UnlockSequence[0]) ? 1 : 0;
	}
}

void UDevToolsSubsystem::OnUnlockSequenceEntered()
{
	// Always (re)open; the widget owns closing (Back / Close button). bClearLayer avoids stacking.
	OpenLevelSelect();
}

void UDevToolsSubsystem::OpenLevelSelect()
{
	const UDevToolsSettings* Settings = UDevToolsSettings::Get();
	if (!Settings)
		return;

	TSubclassOf<UCommonActivatableWidget> WidgetClass = Settings->LevelSelectWidgetClass.LoadSynchronous();
	if (!WidgetClass)
		return;

	UGameInstance* GI = GetGameInstance();
	ULocalPlayer* LP = GI ? GI->GetFirstGamePlayer() : nullptr;
	if (!LP)
		return;

	if (UUISubsystem* UI = LP->GetSubsystem<UUISubsystem>())
	{
		// Dedicated debug layer, pause the game while the menu is up. bClearLayer=true makes
		// re-entering the sequence idempotent (never stacks two copies).
		UI->PushWidgetToLayer(TAG_UI_Layer_Debug, WidgetClass, /*bClearLayer=*/true, /*bPauseGame=*/true);
	}
}

void UDevToolsSubsystem::CloseLevelSelect()
{
	UGameInstance* GI = GetGameInstance();
	ULocalPlayer* LP = GI ? GI->GetFirstGamePlayer() : nullptr;
	if (LP)
	{
		if (UUISubsystem* UI = LP->GetSubsystem<UUISubsystem>())
			UI->ClearLayer(TAG_UI_Layer_Debug);
	}
}

UDataTable* UDevToolsSubsystem::GetChapterTable() const
{
	const UDevToolsSettings* Settings = UDevToolsSettings::Get();
	return Settings ? Settings->ChapterTable.LoadSynchronous() : nullptr;
}

TArray<FDevChapterEntry> UDevToolsSubsystem::GetChapters() const
{
	TArray<FDevChapterEntry> Result;

	UDataTable* Table = GetChapterTable();
	if (!Table)
		return Result;

	// Resolve name + image from the shared checkpoint display DataTable (same source the real
	// save-slot buttons use), letting per-row overrides win when set.
	USaveSubsystem* SaveSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveSubsystem>() : nullptr;

	TArray<int32> SortOrders;
	Table->ForeachRow<FDevChapterRow>(TEXT("DevTools_GetChapters"),
		[&Result, &SortOrders, SaveSub](const FName& RowName, const FDevChapterRow& Row)
		{
			FDevChapterEntry Entry;
			Entry.RowName = RowName;
			if (SaveSub && !Row.CheckpointId.IsNone())
				SaveSub->GetCheckpointDisplayData(Row.CheckpointId, Entry.DisplayName, Entry.Thumbnail);

			Result.Add(MoveTemp(Entry));
			SortOrders.Add(Row.SortOrder);
		});

	// Stable sort by the parallel SortOrder array.
	TArray<int32> Indices;
	Indices.Reserve(Result.Num());
	for (int32 i = 0; i < Result.Num(); ++i) Indices.Add(i);
	Indices.StableSort([&SortOrders](int32 A, int32 B) { return SortOrders[A] < SortOrders[B]; });

	TArray<FDevChapterEntry> Sorted;
	Sorted.Reserve(Result.Num());
	for (int32 Idx : Indices) Sorted.Add(MoveTemp(Result[Idx]));
	return Sorted;
}

bool UDevToolsSubsystem::SkipToChapter(FName RowName)
{
	UDataTable* Table = GetChapterTable();
	if (!Table)
		return false;

	const FDevChapterRow* Row = Table->FindRow<FDevChapterRow>(RowName, TEXT("DevTools_SkipToChapter"));
	if (!Row)
		return false;

	UDevChapterSaveData* SaveData = Row->SaveData.LoadSynchronous();
	if (!SaveData || SaveData->SaveBytes.Num() == 0)
		return false;

	UGameInstance* GI = GetGameInstance();
	USaveSubsystem* SaveSub = GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;
	if (!SaveSub)
		return false;

	CloseLevelSelect();
	return SaveSub->SkipToChapterFromBytes(SaveData->SaveBytes);
}
