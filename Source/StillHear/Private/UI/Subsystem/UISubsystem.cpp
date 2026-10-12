#include "UI/Subsystem/UISubsystem.h"

#include "UI/Widgets/PrimaryGameLayout.h"

#pragma region METHODS
void UUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUISubsystem::Deinitialize()
{
	RootLayout = nullptr;
	Super::Deinitialize();
}
#pragma endregion

#pragma region UFUNCTIONS
void UUISubsystem::SetPrimaryLayout(UPrimaryGameLayout* InLayout)
{
	RootLayout = InLayout;
	
	if (RootLayout)
		OnUILayoutReady.Broadcast();
}

UPrimaryGameLayout* UUISubsystem::GetPrimaryLayout() const
{
	return RootLayout;
}

UCommonActivatableWidget* UUISubsystem::PushWidgetToLayer(const FGameplayTag LayerTag, const TSubclassOf<UCommonActivatableWidget> WidgetClass, const bool bClearLayer, const bool bPauseGame)
{
	if (!RootLayout)
	{
		UE_LOG(LogTemp, Warning, TEXT("UISubsystem: Cannot push widget, RootLayout is null"));
		return nullptr;
	}

	return RootLayout->PushWidgetToLayer(LayerTag, WidgetClass, bClearLayer, bPauseGame);
}

void UUISubsystem::ClearLayer(const FGameplayTag LayerTag)
{
	if (!RootLayout)
	{
		UE_LOG(LogTemp, Warning, TEXT("UISubsystem: Cannot clear layer, RootLayout is null"));
		return;
	}

	RootLayout->ClearLayer(LayerTag);
}
#pragma endregion
