#include "UI/Subsystem/CustomHUD.h"

#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/PrimaryGameLayout.h"

#pragma region METHODS

void ACustomHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = GetOwningPlayerController();
	if (!PrimaryLayoutClass || !PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("CustomHUD: Missing PrimaryLayoutClass or PlayerController"));
		return;
	}

	// Create the physical Root Layout widget
	SpawnedLayout = CreateWidget<UPrimaryGameLayout>(PC, PrimaryLayoutClass);
	if (SpawnedLayout)
	{
		// Add the layout to the viewport
		SpawnedLayout->AddToViewport();

		// IMPORTANT: Inform the subsystem that we have a layout!
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>())
			{
				UISubsystem->SetPrimaryLayout(SpawnedLayout);
			}
		}
	}
}

void ACustomHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (const APlayerController* PC = GetOwningPlayerController())
	{
		if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>())
			{
				UISubsystem->SetPrimaryLayout(nullptr);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}
#pragma endregion
