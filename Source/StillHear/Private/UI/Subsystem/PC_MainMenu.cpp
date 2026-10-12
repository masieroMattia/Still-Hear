#include "UI/Subsystem/PC_MainMenu.h"

#include "CommonActivatableWidget.h"
#include "UI/Subsystem/UISubsystem.h"
#include "SaveSystem/SaveSubsystem.h"

#pragma region METHODS
void APC_MainMenu::BeginPlay()
{
	Super::BeginPlay();

	// If we are loading a save game directly (not booting to the menu), skip pushing the menu widget
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
		{
			if (SaveSub->GetCurrentSlotSave() > 0)
			{
				return;
			}
		}
	}

	// Get the local player to access the UI subsystem
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
		return;

	// Retrieve the UI Subsystem from the local player
	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
		return;

	// Check if the layout is already initialized to avoid race conditions
	if (UISubsystem->GetPrimaryLayout())
		PushMainMenuWidget();
	else
		UISubsystem->OnUILayoutReady.AddDynamic(this, &APC_MainMenu::PushMainMenuWidget);
}

void APC_MainMenu::PushMainMenuWidget()
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
		return;

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
		return;

	if (!MainMenuWidgetClass || !MainMenuLayerTag.IsValid())
		return;

	// Push the widget to the designated layer
	UISubsystem->PushWidgetToLayer(MainMenuLayerTag, MainMenuWidgetClass, false);

	bShowMouseCursor = true;
}
#pragma endregion