#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/PopupWidget.h"
#include "ImagePopupWidget.generated.h"

class UImage;
class UTexture2D;
class UMaterialInterface;

UCLASS(Abstract)
class STILLHEAR_API UImagePopupWidget : public UPopupWidget
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Image component for the visual (Optional, in case you don't use it)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ImageDisplay;
#pragma endregion

#pragma region UFUNCTIONS
public:
	// Initialize the widget with a text and an optional texture or material
	UFUNCTION(BlueprintCallable, Category = "Image Popup UI")
	virtual void InitializeImagePopup(const FText& Text, float Duration = 0.0f, TSoftObjectPtr<UTexture2D> Texture = nullptr, TSoftObjectPtr<UMaterialInterface> Material = nullptr);
#pragma endregion
};
