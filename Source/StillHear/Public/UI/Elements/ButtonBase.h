#pragma once

#include "UI/UIEnum.h"
#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "GameplayTagContainer.h"
#include "ButtonBase.generated.h"

class UImage;
class UCommonTextBlock;

UCLASS()
class STILLHEAR_API UButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, Category = "Button|Action", meta = (Categories = "UI.Action"))
	FGameplayTag ButtonActionTag;
	
	// Determines the visual layout of the button
	UPROPERTY(EditAnywhere, Category = "Button|Style")
	EButtonContentMode ContentMode = EButtonContentMode::TextOnly;
	
	// Determines if the input action icon should be visible
	UPROPERTY(EditAnywhere, Category = "Button|Input")
	bool bShowInputAction = true;
	
	UPROPERTY(EditAnywhere, Category = "Button|Input")
	bool bDisableFocusOnGamepad = false;

	UPROPERTY(EditAnywhere, Category = "Button|Input")
	bool bFocusOnHover = true;

	UPROPERTY(EditAnywhere, Category = "Button|Input")
	bool bShowInputActionOnlyOnGamepad = true;
	
	UPROPERTY(EditAnywhere, Category = "Button|Text", meta = (EditCondition = "ContentMode == EButtonContentMode::TextOnly || ContentMode == EButtonContentMode::TextAndIcon", EditConditionHides))
	FText ButtonText;
	
	UPROPERTY(EditAnywhere, Category = "Button|Icon", meta = (EditCondition = "ContentMode == EButtonContentMode::IconOnly || ContentMode == EButtonContentMode::TextAndIcon", EditConditionHides))
	TObjectPtr<UTexture2D> ButtonIcon;
		
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HoveredAnimation;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> PressedAnimation;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> DisabledAnimation;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ButtonLabel;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> FocusIndicator;
#pragma endregion
	
#pragma region METHODS
public:
	// Injects dynamic content into the button and refreshes the UI
	void SetButtonContent(const FText& InText, UTexture2D* InIcon);
	
	virtual void UpdateInputActionWidget() override;

	void SetIsPersistentBinding(bool bInIsPersistentBinding);

	virtual void UpdateFocusIndicator();
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	
	//Called when the button's hovered state changes
	virtual void NativeOnHovered() override;
	//Called when the button's unhovered state changes
	virtual void NativeOnUnhovered() override;
	// Called when the button's released state changes
	virtual void NativeOnReleased() override;
	virtual void NativeOnClicked() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnSelected(bool bBroadcast) override;
	virtual void NativeOnDeselected(bool bBroadcast) override;

	void HandleActionClick();
	void RefreshButtonContent() const;
	
	// Animations
	void PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode);
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Button|Style")
	void UpdateButtonStyle();
#pragma endregion

protected:
	ECommonInputType LastInputType = static_cast<ECommonInputType>(0);

	uint64 LastPressedAnimationPlayFrame = 0;
	float CreationTimeSeconds = 0.0f;


	void PlayPressedAnimation();
};
