// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedActionKeyMapping.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputSubsystem.generated.h"

enum class EControllerCategory : uint8;
enum class EKeyboardMoveDirection : uint8;
enum class EInputDeviceType : uint8;
/**
 * 
 */
UCLASS()
class STILLHEAR_API UInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY()
	TArray<TObjectPtr<class UInputMappingContext>> AllMappingContexts;
	UPROPERTY()
	TArray<struct FBindingData> CurrentBindings;
	UPROPERTY()
	TArray<struct FBindingData> RebindUtilityArray; // Used to store temporary data during rebinding process
	UPROPERTY()
	EControllerCategory CurrentControllerType;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	UInputSubsystem();
#pragma endregion 
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Controller")
	void SetControllerInputType(const FName& ControllerName);

	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void SaveBindings();
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void DeleteBindings();
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void ResetSavedBindingsToDefault();
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void ApplyDefaultBindings();
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void AddToKeysToRebind(const FBindingData& BindingData);
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void AddToKeysToRebindArray(TArray<FBindingData>& Bindings);
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void ClearKeysToRebind();
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	void RebindKeys();

	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	FKey GetCurrentKeyForAction(class UInputAction* InputAction, const EInputDeviceType DeviceType);
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	FKey GetDefaultKeyForAction(class UInputAction* InputAction, const FKey CurrentKey);
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	FKey GetCurrentKeyForMoveDirection(class UInputAction* InputAction, const EKeyboardMoveDirection Direction);
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	FSlateBrush GetBrushFromKey(const FKey Key, const EInputDeviceType DeviceType) const;
	UFUNCTION(BlueprintCallable, Category = "Input Bindings")
	bool IsInputActionSetToDefault(UInputAction* InputAction, const FKey& Key);
#pragma endregion 
	
#pragma region METHODS
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void CacheDefaultBindings();
	void ApplySavedBindings(const class USettingsSaveGame* Settings);

	bool HasNegateModifier(const FEnhancedActionKeyMapping& Mapping);
	bool HasSwizzleModifier(const FEnhancedActionKeyMapping& Mapping);
#pragma endregion
};
