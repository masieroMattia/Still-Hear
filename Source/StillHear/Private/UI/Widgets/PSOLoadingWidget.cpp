#include "UI/Widgets/PSOLoadingWidget.h"

#include "CommonTextBlock.h"
#include "Data/DataTables/PSOMessageData.h"
#include "FunctionLibrary/PSOBlueprintLibrary.h"

#pragma region METHODS
void UPSOLoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitialPSOCount = UPSOBlueprintLibrary::GetPSORemaining();

	// Cache all messages from the DataTable at start
	if (LoadingMessagesTable)
		LoadingMessagesTable->GetAllRows<FPSOMessageData>(TEXT("PSO Loading Context"), CachedMessages);
}

void UPSOLoadingWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (UPSOBlueprintLibrary::IsPSOCompilationComplete())
	{
		OnCompilationFinishedDelegate.Broadcast();
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UpdateCurrentMessage(InDeltaTime);

	const int32 CurrentRemaining = UPSOBlueprintLibrary::GetPSORemaining();
	float ProgressPercentage = 0.0f;
	
	if (InitialPSOCount > 0)
		ProgressPercentage = FMath::RoundToInt(100.0f * (1.0f - (static_cast<float>(CurrentRemaining) / InitialPSOCount)));

	UpdateUI(ProgressPercentage, GetDisplayString());
}

void UPSOLoadingWidget::UpdateCurrentMessage(const float DeltaTime)
{
	if (CachedMessages.Num() == 0) 
		return;

	MessageTimer += DeltaTime;
	if (MessageTimer >= MessageChangeInterval)
	{
		MessageTimer = 0.0f;
		// Cycle to next row, loop back if at end
		CurrentRowIndex = (CurrentRowIndex + 1) % CachedMessages.Num();
	}
}

FString UPSOLoadingWidget::GetDisplayString()
{
	// Handle dots animation
	DotTimer += GetWorld()->GetDeltaSeconds();
	if (DotTimer > 0.5f)
	{
		DotTimer = 0.0f;
		DotCount = (DotCount + 1) % 4;
	}

	FString Dots = TEXT("");
	for (int32 i = 0; i < DotCount; ++i) 
		Dots += TEXT(".");

	// Combine table message with dots
	const FString BaseMessage = CachedMessages.IsValidIndex(CurrentRowIndex) ? CachedMessages[CurrentRowIndex]->Message : TEXT("Loading");
	return FString::Printf(TEXT("%s%s"), *BaseMessage, *Dots);
}

void UPSOLoadingWidget::UpdateUI(const int32 Percentage, const FString& LoadingText) const
{
	if (PercentageText)
	{
		// Format the string and convert to FTextf
		const FString FormattedPercentage = FString::Printf(TEXT("%d %%"), Percentage);
		PercentageText->SetText(FText::FromString(FormattedPercentage));
	}

	if (MessageText)
		MessageText->SetText(FText::FromString(LoadingText));
}
#pragma endregion