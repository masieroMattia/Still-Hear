#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonInputTypeEnum.h"
#include "InputActionWidget.generated.h"

class UCommonActionWidget;
class UWidgetAnimation;
class UInputAction;
class USizeBox;
class UImage;

USTRUCT(BlueprintType)
struct FCustomIconData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TObjectPtr<UTexture2D> IconTexture;

	// The custom size to apply to the image widget when using this override
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FVector2D IconSize = FVector2D(128.0f, 64.0f);
};

/**
 * Base class for dynamic action widgets. Handles robust animation state logic
 */
UCLASS(Abstract)
class STILLHEAR_API UInputActionWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// Automatically binds to a CommonActionWidget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tutorial")
	TObjectPtr<UCommonActionWidget> ActionIcon;

	// Optional size box wrapping the icons, used to change dimensions dynamically
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tutorial")
	TObjectPtr<USizeBox> IconSizeBox;

	// Optional custom image widget for overrides
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tutorial")
	TObjectPtr<UImage> CustomIcon;

	// Map of InputActions that should use a custom texture instead of the default CommonUI glyph
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	TMap<TObjectPtr<UInputAction>, FCustomIconData> CustomActionIcons;

	// --- ANIMATIONS ---
	// Looping animation when the action is not pressed
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> NormalIdleAnim;

	// The press transition animation
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> PressAnim;

	// Looping animation when the action is held down
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> PressedIdleAnim;
#pragma endregion

#pragma region VARIABLES
private:
	// Tracks the current press state of the action
	bool bIsPressed = false;

	// The currently bound input action
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BoundAction;

	// Cached SizeBox values
	float DefaultWidthOverride = 0.0f;
	float DefaultHeightOverride = 0.0f;
	
	bool bHasDefaultWidthOverride = false;
	bool bHasDefaultHeightOverride = false;
	
	bool bHasCachedSizeBox = false;
#pragma endregion

#pragma region UFUNCTIONS
public:
	// Automatically binds the InputAction to the ActionIcon widget
	UFUNCTION(BlueprintNativeEvent, Category = "Tutorial")
	void SetupInputAction(UInputAction* InputAction);
	
	// Drives the animation forward
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void PressAction();

	// Drives the animation backward
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ReleaseAction();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeConstruct() override;

	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UFUNCTION()
	void OnInputMethodChanged(ECommonInputType CurrentInputType);
#pragma endregion
};
