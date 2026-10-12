#include "UI/Widgets/ImagePopupWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"

void UImagePopupWidget::InitializeImagePopup(const FText& Text, float Duration, TSoftObjectPtr<UTexture2D> Texture, TSoftObjectPtr<UMaterialInterface> Material)
{
	// Initialize the base popup properties (Message and Duration)
	InitializePopup(Text, Duration);

	// Set the image or material (Optional)
	if (ImageDisplay)
	{
		if (!Texture.IsNull())
		{
			if (UTexture2D* LoadedTexture = Texture.LoadSynchronous())
			{
				ImageDisplay->SetBrushFromTexture(LoadedTexture);
			}
		}
		else if (!Material.IsNull())
		{
			if (UMaterialInterface* LoadedMaterial = Material.LoadSynchronous())
			{
				ImageDisplay->SetBrushFromMaterial(LoadedMaterial);
			}
		}
	}
}
