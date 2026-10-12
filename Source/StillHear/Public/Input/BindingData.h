
#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputDeviceType.h"
#include "BindingData.generated.h"

enum class EInputDeviceType : uint8;

/**
* The following struct is used to handle inputs settings
 * such as rebinding keys and input actions. They are stored in the SettingsSaveGame class.
 */
USTRUCT(BlueprintType)
struct FBindingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey CurrentBoundKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey DefaultBoundKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EInputDeviceType DeviceType = EInputDeviceType::KeyboardMouse;

	bool operator==(const FBindingData& Other) const
	{
		return InputAction == Other.InputAction &&
			   CurrentBoundKey == Other.CurrentBoundKey &&
			   DefaultBoundKey == Other.DefaultBoundKey &&
			   DeviceType == Other.DeviceType;	
	}
};