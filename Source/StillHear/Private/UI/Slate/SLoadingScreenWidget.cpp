#include "UI/Slate/SLoadingScreenWidget.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProjectSettings/LoadingScreenSettings.h"

// Static singleton
TSharedPtr<SLoadingScreenWidget> SLoadingScreenWidget::Instance = nullptr;

#pragma region CONSTRUCTOR
void SLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	const ULoadingScreenSettings* Settings = InArgs._Settings;
	const bool bIsDeathScreen = InArgs._bIsDeathScreen;
	
	if (!Settings) 
	{
		Settings = GetDefault<ULoadingScreenSettings>();
	}

	// Resolve Context-Specific Properties
	FLinearColor BGColor = bIsDeathScreen ? Settings->DeathBackgroundColor : Settings->BackgroundColor;
	UMaterialInterface* Material = bIsDeathScreen ? Settings->DeathMaterial.LoadSynchronous() : Settings->LoadingMaterial.LoadSynchronous();
	const FString DisplayTextStr = bIsDeathScreen ? Settings->DeathText : Settings->LoadingText;

	// Build the Icon
	TSharedPtr<SWidget> IconWidget;

	if (Material)
	{
		// Create a dynamic material instance for runtime parameter changes
		MaterialInstance = UMaterialInstanceDynamic::Create(Material, nullptr);

		// Set up an FSlateBrush that references the material
		MaterialBrush = MakeUnique<FSlateBrush>();
		MaterialBrush->SetResourceObject(MaterialInstance);
		MaterialBrush->ImageSize = Settings->IconSize;
		MaterialBrush->DrawAs = ESlateBrushDrawType::Image;

		IconWidget = SNew(SBox)
			.WidthOverride(Settings->IconSize.X)
			.HeightOverride(Settings->IconSize.Y)
			[
				SNew(SImage).Image(MaterialBrush.Get())
			];
	}
	else
	{
		// Fallback: circular throbber when no material is set
		IconWidget = SNew(SCircularThrobber).Radius(Settings->ThrobberFallbackRadius);
	}

	// Build the Text Widget (if any)
	TSharedPtr<SWidget> TextWidget;
	if (!DisplayTextStr.IsEmpty())
	{
		TextWidget = SNew(STextBlock)
			.Text(FText::FromString(DisplayTextStr))
			.Font(Settings->TextFont.HasValidFont() ? Settings->TextFont : FCoreStyle::GetDefaultFontStyle("Regular", 36))
			.ColorAndOpacity(Settings->TextColor);
	}
	else
	{
		TextWidget = SNew(SBox); // Empty box if no text
	}

	// Compose the Full Loading/Death Screen
	ChildSlot
	[
		SNew(SOverlay)
		// Layer 1: Full-screen background
		+ SOverlay::Slot()
		[
			SNew(SColorBlock).Color(BGColor)
		]
		// Layer 2: Optional text
		+ SOverlay::Slot()
		.HAlign(Settings->TextHorizontalAlignment)
		.VAlign(Settings->TextVerticalAlignment)
		[
			TextWidget.ToSharedRef()
		]
		// Layer 3: Animated icon with custom alignment
		+ SOverlay::Slot()
		.HAlign(Settings->IconHorizontalAlignment)
		.VAlign(Settings->IconVerticalAlignment)
		.Padding(Settings->IconPadding)
		[
			IconWidget.ToSharedRef()
		]
	];
}

#pragma endregion

#pragma region METHODS
void SLoadingScreenWidget::Show(const bool bIsDeathScreen, const ULoadingScreenSettings* Settings)
{
	if (Instance.IsValid())
		return;

	if (!GEngine || !GEngine->GameViewport)
		return;

	if (!Settings)
	{
		Settings = GetDefault<ULoadingScreenSettings>();
	}

	Instance = SNew(SLoadingScreenWidget)
		.Settings(Settings)
		.bIsDeathScreen(bIsDeathScreen);

	GEngine->GameViewport->AddViewportWidgetContent(Instance.ToSharedRef(), MAX_int32);
}

void SLoadingScreenWidget::Hide()
{
	if (!Instance.IsValid())
		return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Instance.ToSharedRef());
	}

	Instance.Reset();
}

bool SLoadingScreenWidget::IsActive()
{
	return Instance.IsValid();
}

void SLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (MaterialInstance)
	{
		Collector.AddReferencedObject(MaterialInstance);
	}
}

#pragma endregion
