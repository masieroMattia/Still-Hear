#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "PressAnyKeyWidget.generated.h"

enum class EInputDeviceType : uint8;
class FPressAnyKeyInputProcessor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRebindKeySelected, const FKey&, SelectedKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRebindCanceled);

UCLASS()
class STILLHEAR_API UPressAnyKeyWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnRebindKeySelected OnKeySelected;

	UPROPERTY(BlueprintAssignable)
	FOnRebindCanceled OnCanceled;

	// This is called just after this widget is activated, just to understand what device we are mapping
	// for future logic, in case we should remap the gamepad.
	void InitializeForDevice(EInputDeviceType InDeviceType);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	void HandleKeySelected(const FKey& Key);
	void HandleCanceled();

	TSharedPtr<FPressAnyKeyInputProcessor> InputProcessor;
	bool bAcceptGamepadKeys = false;
};
