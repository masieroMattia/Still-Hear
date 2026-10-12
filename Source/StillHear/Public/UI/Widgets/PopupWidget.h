#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UIWidgetInterface.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "PopupWidget.generated.h"

class UCommonTextBlock;
class UInputAction;

UCLASS(Abstract)
class STILLHEAR_API UPopupWidget : public UActivatableWidgetBase, public IUIWidgetInterface
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Popup")
	FText PopupMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", UIMin = "0.1"), Category = "UI|Popup")
	float PopupDuration = 3.0f;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> MessageText;
#pragma endregion

#pragma region VARIABLES
private:
	FTimerHandle LifetimeTimerHandle;
#pragma endregion
	
#pragma region METHODS
protected:
	// Called automatically when the widget is added to a layer and activated
	virtual void NativeOnActivated() override;
	// Called automatically when the widget is deactivated or removed
	virtual void NativeOnDeactivated() override;
	// Called when the lifetime of the widget expires
	void OnLifetimeExpired();
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable)
	void InitializePopup(const FText& InMessage, float InDuration);
#pragma endregion
	
#pragma region INTERFACE METHODS
public:
	// IUIWidgetInterface implementation
	virtual void InitializeWidget_Implementation(const FText& Text, float Duration, const TArray<UInputAction*>& InputActions) override;
#pragma endregion
};
