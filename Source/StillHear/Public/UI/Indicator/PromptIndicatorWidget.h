#pragma once

#include "CoreMinimal.h"
#include "IndicatorWidgetBase.h"
#include "PromptIndicatorWidget.generated.h"

class UHorizontalBox;
class UInputActionWidget;
enum class ECommonInputType : uint8;

UCLASS(Abstract)
class STILLHEAR_API UPromptIndicatorWidget : public UIndicatorWidgetBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ActionContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	TSubclassOf<UInputActionWidget> InputActionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	TSubclassOf<UUserWidget> SeparatorWidgetClass;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
#pragma endregion

#pragma region UFUNCTIONS
protected:
	// Automatically rebuilds the UI based on Descriptor->InputActions
	UFUNCTION()
	void UpdateInputPrompt();
#pragma endregion
};
