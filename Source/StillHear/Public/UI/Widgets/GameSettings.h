// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Elements/TabWidgetBase.h"
#include "GameSettings.generated.h"

/**
 * 
 */

UCLASS()
class STILLHEAR_API UGameSettings : public UTabWidgetBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UGraphicsSettingsWidget> GraphicsSettingsWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UAudioSettingsWidget> AudioSettingsWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UControlsSettingsWidget> ControlsSettingsWidget;
#pragma endregion

#pragma region VARIABLES
private:
	bool bInterceptBackInputAction = false;
#pragma endregion 

#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void DeactivateBackInputAction();

	UFUNCTION()
	void ActivateBackInputAction();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void HandleBackActionBinding() override;
	
	virtual void HandleSwitcherTransitionFinished(bool bIsTransitioning) override;
#pragma endregion 
};
