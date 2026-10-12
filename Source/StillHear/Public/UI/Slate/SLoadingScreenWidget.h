#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMaterialInstanceDynamic;
class ULoadingScreenSettings;
class UMaterialInterface;

/**
 * Full-screen Slate loading screen widget
 * Displays a black background with an animated material icon in the bottom-right corner
 * Uses pure Slate rendering: survives level transitions and never freezes
 *
 * Usage: call SLoadingScreenWidget::Show() / Hide() from anywhere
 */
class STILLHEAR_API SLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
#pragma region PROPERTIES
	SLATE_BEGIN_ARGS(SLoadingScreenWidget)
		: _Settings(nullptr)
		, _bIsDeathScreen(false)
	{}
		// Full settings object for the loading screen
		SLATE_ARGUMENT(const ULoadingScreenSettings*, Settings)
		// Whether this is a death screen (to use alternate settings)
		SLATE_ARGUMENT(bool, bIsDeathScreen)
	SLATE_END_ARGS()
#pragma endregion

#pragma region VARIABLES
private:
	// Singleton instance managed by Show/Hide
	static TSharedPtr<SLoadingScreenWidget> Instance;

	// Dynamic material instance
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	// Brush used to render the material in Slate
	TUniquePtr<FSlateBrush> MaterialBrush;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	void Construct(const FArguments& InArgs);
#pragma endregion
	
#pragma region METHODS
protected:
	// Prevent GC of the dynamic material instance (from FGCObject)
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("SLoadingScreenWidget"); }

public:
	// ── Static API
	// Shows the loading screen
	static void Show(bool bIsDeathScreen = false, const ULoadingScreenSettings* Settings = nullptr);

	// Hides the loading screen
	static void Hide();

	// Returns true if the loading screen is currently active
	static bool IsActive();
#pragma endregion

};
