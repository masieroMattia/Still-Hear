#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/DeveloperSettings.h"
#include "LoadingScreenSettings.generated.h"

/**
 * Project settings for the loading screen
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Loading Screen"))
class STILLHEAR_API ULoadingScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// --- NORMAL LOADING SCREEN ---
	// Background color of the normal loading screen
	UPROPERTY(config, EditAnywhere, Category = "Normal Loading")
	FLinearColor BackgroundColor = FLinearColor::Black;

	// Material for the normal loading screen
	UPROPERTY(config, EditAnywhere, Category = "Normal Loading")
	TSoftObjectPtr<UMaterialInterface> LoadingMaterial;

	// Text to display on the normal loading screen
	UPROPERTY(config, EditAnywhere, Category = "Normal Loading")
	FString LoadingText = TEXT("Loading...");

	// --- DEATH SCREEN ---
	// Background color of the death screen
	UPROPERTY(config, EditAnywhere, Category = "Death Screen")
	FLinearColor DeathBackgroundColor = FLinearColor(0.1f, 0.0f, 0.0f, 1.0f); // Dark red default

	// Material for the death screen
	UPROPERTY(config, EditAnywhere, Category = "Death Screen")
	TSoftObjectPtr<UMaterialInterface> DeathMaterial;

	// Text to display on the death screen
	UPROPERTY(config, EditAnywhere, Category = "Death Screen")
	FString DeathText = TEXT("You Died");

	// Delay (in seconds) before the death screen appears
	UPROPERTY(config, EditAnywhere, Category = "Death Screen")
	float DeathScreenInitialDelay = 2.0f;

	// How long (in seconds) the death screen should remain visible before starting the respawn
	UPROPERTY(config, EditAnywhere, Category = "Death Screen")
	float DeathScreenDuration = 3.0f;

	// --- SHARED ICON SETTINGS ---
	// Size of the animated icon in pixels
	UPROPERTY(config, EditAnywhere, Category = "Shared Icon Settings")
	FVector2D IconSize = FVector2D(128.f, 128.f);

	// Horizontal alignment of the icon
	UPROPERTY(config, EditAnywhere, Category = "Shared Icon Settings")
	TEnumAsByte<EHorizontalAlignment> IconHorizontalAlignment = HAlign_Right;

	// Vertical alignment of the icon
	UPROPERTY(config, EditAnywhere, Category = "Shared Icon Settings")
	TEnumAsByte<EVerticalAlignment> IconVerticalAlignment = VAlign_Bottom;

	// Padding from the chosen corner in pixels
	UPROPERTY(config, EditAnywhere, Category = "Shared Icon Settings")
	float IconPadding = 50.f;

	// Radius of the circular throbber used as fallback
	UPROPERTY(config, EditAnywhere, Category = "Shared Icon Settings")
	float ThrobberFallbackRadius = 25.f;

	// --- TEXT STYLE ---
	// Font used for the text
	UPROPERTY(config, EditAnywhere, Category = "Text Style")
	FSlateFontInfo TextFont;

	// Color of the text
	UPROPERTY(config, EditAnywhere, Category = "Text Style")
	FLinearColor TextColor = FLinearColor::White;

	// Horizontal alignment of the text
	UPROPERTY(config, EditAnywhere, Category = "Text Style")
	TEnumAsByte<EHorizontalAlignment> TextHorizontalAlignment = HAlign_Center;

	// Vertical alignment of the text
	UPROPERTY(config, EditAnywhere, Category = "Text Style")
	TEnumAsByte<EVerticalAlignment> TextVerticalAlignment = VAlign_Center;

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
