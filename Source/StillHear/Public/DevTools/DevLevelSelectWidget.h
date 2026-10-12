#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "DevLevelSelectWidget.generated.h"

class UPanelWidget;
class USaveSlotButtonBase;

/**
 * DEV TOOL — the level-skip menu, built in C++ so it can reuse the existing save-slot buttons
 * (USaveSlotButtonBase) and therefore show the same checkpoint thumbnail / name as the real
 * save slots. Pushed onto the debug UI layer by UDevToolsSubsystem when the unlock sequence fires.
 */
UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UDevLevelSelectWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

protected:
	// Container the chapter buttons are added to (a VerticalBox / ScrollBox placed in the WBP).
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ChapterList;

	// Button widget class used for each chapter — reuse your existing save-slot button WBP
	// (or a variant of it) so the look matches the real save menu.
	UPROPERTY(EditDefaultsOnly, Category = "Dev Level Select")
	TSubclassOf<USaveSlotButtonBase> ChapterButtonClass;

	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void Populate();

	void HandleChapterClicked(FName RowName);

	UPROPERTY(Transient)
	TObjectPtr<USaveSlotButtonBase> FirstButton;
};
