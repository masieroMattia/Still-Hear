#pragma once

#include "CoreMinimal.h"
#include "ButtonBase.h"
#include "PopupButtonBase.generated.h"

class UPopupWidget;

UCLASS()
class STILLHEAR_API UPopupButtonBase : public UButtonBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, Category = "Button|Popup")
	TSubclassOf<UPopupWidget> PopupClass;

	UPROPERTY(EditAnywhere, meta = (Categories = "UI.Layer"), Category= "Button|Popup")
	FGameplayTag PopupLayerTag;

	UPROPERTY(EditAnywhere, Category = "Button|Popup")
	FText PopupMessage;

	UPROPERTY(EditAnywhere, Category = "Button|Popup")
	float PopupDuration = 3.0f;
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeOnInitialized() override;

private:
	void ShowPopup() const;
#pragma endregion
};
