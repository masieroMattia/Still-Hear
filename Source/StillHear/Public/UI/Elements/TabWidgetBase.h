#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/ActivatableWidgetBase.h"
#include "TabWidgetBase.generated.h"

class UCommonAnimatedSwitcher;
class UCommonTabListWidgetBase;

// Struct to hold the configuration for each tab
USTRUCT(BlueprintType)
struct FTabDisplayData
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FText TabName;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	TObjectPtr<UTexture2D> TabIcon;
};

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UTabWidgetBase : public UActivatableWidgetBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTabListWidgetBase> TabButtonsContainer;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> ContentSwitcher;

	// The button class to spawn for each tab
	UPROPERTY(EditDefaultsOnly, Category = "Tabs")
	TSubclassOf<UButtonBase> TabButtonClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tabs")
	TArray<FTabDisplayData> TabsDisplayData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> PrevTabButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButtonBase> NextTabButton;

	UPROPERTY(EditDefaultsOnly, Category = "Tabs|Input")
	FDataTableRowHandle PrevTabInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Tabs|Input")
	FDataTableRowHandle NextTabInputAction;
#pragma endregion
    
#pragma region UFUNCTIONS
private:
	UFUNCTION()
	void HandlePrevTabClicked();

	UFUNCTION()
	void HandleNextTabClicked();
#pragma endregion
    
#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual void HandleSwitcherTransitionFinished(bool bIsTransitioning);
#pragma endregion
};