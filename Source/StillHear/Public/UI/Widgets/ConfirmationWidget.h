#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "ConfirmationWidget.generated.h"

class UCommonTextBlock;
class UButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmationResult);

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UConfirmationWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> MessageText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> CancelButton;

	UPROPERTY(EditDefaultsOnly, Category = "Confirmation|Input")
	FDataTableRowHandle ConfirmInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Confirmation|Input")
	FDataTableRowHandle CancelInputAction;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Confirmation")
	FOnConfirmationResult OnConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Confirmation")
	FOnConfirmationResult OnCancelled;
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Confirmation")
	void InitializeConfirmation(const FText& Message);
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	FUIActionBindingHandle ConfirmBindingHandle;
	FUIActionBindingHandle CancelBindingHandle;

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCancelClicked();

	void ExecuteConfirm();
	void ExecuteCancel();
#pragma endregion
};
