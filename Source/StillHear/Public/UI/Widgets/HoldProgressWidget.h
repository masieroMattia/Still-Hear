#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "HoldProgressWidget.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UInputActionWidget;
class UInputAction;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldComplete);

UCLASS(Abstract)
class STILLHEAR_API UHoldProgressWidget : public UActivatableWidgetBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Hold Progress")
	TObjectPtr<UImage> ProgressRing;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Hold Progress")
	TObjectPtr<UInputActionWidget> InputActionIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Hold Progress")
	TObjectPtr<UMaterialInterface> ProgressMaterial;

	// The Enhanced Input Action used to detect keys and display the correct UI icon
	UPROPERTY(EditDefaultsOnly, Category = "Hold Progress")
	TObjectPtr<UInputAction> HoldAction;

	UPROPERTY(EditDefaultsOnly, Category = "Hold Progress", meta = (ClampMin = "0.1"))
	float HoldDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Hold Progress")
	FName ProgressParameterName = FName("Progress");

	// Seconds of inactivity before the widget automatically hides
	UPROPERTY(EditDefaultsOnly, Category = "Hold Progress", meta = (ClampMin = "0.1"))
	float AutoHideDelay = 3.0f;
#pragma endregion

#pragma region DELEGATES
public:
	UPROPERTY(BlueprintAssignable, Category = "Hold Progress")
	FOnHoldComplete OnHoldComplete;
#pragma endregion

#pragma region VARIABLES
private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	float CurrentProgress = 0.0f;
	bool bIsHolding = false;
	bool bCompleted = false;

	FTimerHandle AutoHideTimer;
	FUIActionBindingHandle ActionBindingHandle;
	TSharedPtr<IInputProcessor> AnyKeyListener;
	TArray<FKey> MappedKeysCache;
#pragma endregion

#pragma region METHODS
public:
	// Called by the custom IInputProcessor when any key is pressed to reveal the widget
	void NotifyAnyInput();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void ApplyProgress(float Progress) const;
	void OnAutoHideTimerExpired();

	// CommonUI Action Binding callbacks
	void OnHoldPressed();
	void OnHoldReleased();
	void OnHoldCompleted();
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Hold Progress")
	void ResetProgress();

	UFUNCTION(BlueprintPure, Category = "Hold Progress")
	float GetCurrentProgress() const { return CurrentProgress; }
#pragma endregion
};
