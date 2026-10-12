// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Keyboard/BindingsPageWidgetBase.h"
#include "ControlsSettingsWidget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBindingsPageOpened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBindingsPageClosed);

UCLASS()
class STILLHEAR_API UControlsSettingsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class USizeBox> ButtonsContainer;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButtonBase> ControllerButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButtonBase> KeyboardButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UBindingsPageWidgetBase> KeyboardBindingsPage;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UGamepadBindingsWidget> GamepadBindingsWidget;
#pragma endregion 

#pragma region VARIABLES
public:
	UPROPERTY()
	FOnBindingsPageOpened OnBindingsPageOpened;

	UPROPERTY()
	FOnBindingsPageClosed OnBindingsPageClosed;
private:
	UPROPERTY()
	TObjectPtr<class UInputSubsystem> InputSubsystem;

	UPROPERTY()
	TObjectPtr<class USaveSubsystem> SaveSubsystem;
#pragma endregion 
	
#pragma region CONSTRUCTOR
protected:
	virtual void NativeConstruct() override;
#pragma endregion

#pragma region METHODS
public:
	void CloseBindingsPage() const;
	bool IsBindingsPageOpen() const;

private:
	void ShowControllerControls() const;
	void ShowKeyboardControls() const;
	void HideMainButtons() const;
#pragma endregion 
};
