// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorizontalBoxButton.h"
#include "InputAction.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "BindingRowWidget.generated.h"

enum class EInputDeviceType : uint8;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeyIsNotDefault);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRebindRequested, UBindingRowWidget*, Row, UInputAction*, InputAction, const FKey&, CurrentKey, const EInputDeviceType, DeviceType);

/**
 * 
 */
UCLASS()
class STILLHEAR_API UBindingRowWidget : public UButtonBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(BlueprintAssignable)
	FOnKeyIsNotDefault OnKeyIsNotDefault;
	UPROPERTY(BlueprintAssignable)
	FOnRebindRequested OnRebindRequested;
	
protected:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ActionNameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> KeyGlyph;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Configuration|Device")
	EInputDeviceType DeviceType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Text")
	FText DisplayText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Input Action")
	TObjectPtr<class UInputAction> InputAction;

	UPROPERTY()
	FKey CurrentKey;
	UPROPERTY()
	FKey ReboundKey;
#pragma endregion
	
#pragma region METHODS
public:
	virtual void SetInitialGlyph();
	void SetGlyphFromKey(const FKey& Key) const;
	FKey GetCurrentKey() const { return CurrentKey; }
	void SetCurrentKey(const FKey& Key) { CurrentKey = Key; }
	FKey GetReboundKey() const { return ReboundKey; }
	void SetReboundKey(const FKey& Key) { ReboundKey = Key; }
	UInputAction* GetInputAction() const { return InputAction; }
	EInputDeviceType GetInputDeviceType() const { return DeviceType; }
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeOnClicked() override;
#pragma endregion 
};
