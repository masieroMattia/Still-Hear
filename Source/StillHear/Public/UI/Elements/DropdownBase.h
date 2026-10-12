#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/DataTable.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "DropdownBase.generated.h"

enum class ECommonInputType : uint8;
class UDropdownMenu;
class UButtonBase;
class UMenuAnchor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDropdownSelectionChanged, int32, SelectedIndex, FText, SelectedText);

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UDropdownBase : public USettingsRowBase
{
	GENERATED_BODY()

protected:
#pragma region UPROPERTIES
	// The available options in the dropdown list
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dropdown")
	TArray<FText> Options;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dropdown")
	int32 CurrentIndex = 0;

	// The widget class to instantiate for the dropdown menu popup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dropdown")
	TSubclassOf<UDropdownMenu> MenuClass;

	// The data table row handle for the Accept/Open action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dropdown|Input")
	FDataTableRowHandle ForwardInputAction;

	// The data table row handle for the Cancel/Close action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dropdown|Input")
	FDataTableRowHandle BackInputAction;

	// Visual bindings
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButtonBase> DropdownButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMenuAnchor> DropdownMenuAnchor;

	// Focus transition animation (optional, matches Rotator style)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FocusAnimation;
#pragma endregion

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Dropdown")
	FOnDropdownSelectionChanged OnSelectionChanged;
#pragma endregion

#pragma region VARIABLES
private:
	bool bIsFocusAnimationActive = false;
	bool bIsClosing = false;
	FUIActionBindingHandle ForwardBindingHandle;

	UPROPERTY()
	TObjectPtr<UDropdownMenu> OpenMenuInstance;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region UFUNCTIONS
public:
	// Populates the dropdown options
	UFUNCTION(BlueprintCallable, Category = "Dropdown")
	void SetOptions(const TArray<FText>& InOptions);

	// Changes the selected option and broadcasts selection change
	UFUNCTION(BlueprintCallable, Category = "Dropdown")
	void SetSelectedIndex(int32 Index);

	int32 GetSelectedIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Dropdown|Input")
	bool IsForwardKey(const FKey& Key) const;

	UFUNCTION(BlueprintPure, Category = "Dropdown|Input")
	bool IsBackKey(const FKey& Key) const;

	UFUNCTION(BlueprintPure, Category = "Dropdown|Input")
	const FDataTableRowHandle& GetForwardInputAction() const { return ForwardInputAction; }

	UFUNCTION(BlueprintPure, Category = "Dropdown|Input")
	const FDataTableRowHandle& GetBackInputAction() const { return BackInputAction; }

	// Opens the dropdown menu using UMenuAnchor
	UFUNCTION(BlueprintCallable, Category = "Dropdown")
	void OpenDropdown();

	// Closes the dropdown menu
	UFUNCTION(BlueprintCallable, Category = "Dropdown")
	void CloseDropdown();
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	
	// Intercept keyboard/gamepad KeyDown to handle "Accept" keys when focused
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;

	// Updates the label text and option text on the main button
	void UpdateVisuals();

	// Bound to DropdownMenuAnchor->OnGetUserMenuContentEvent
	UFUNCTION()
	UUserWidget* HandleGetUserMenuContent();

	// Bind to UMenuAnchor delegates
	UFUNCTION()
	void HandleMenuOpenChanged(bool bIsOpen);

private:
	virtual void RefreshFocusVisuals() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;

	void PlayReversibleAnimation(UWidgetAnimation* InAnimation, EUMGSequencePlayMode::Type PlayMode);
#pragma endregion
};