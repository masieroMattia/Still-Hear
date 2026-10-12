// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BindingsListWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFoundNotDefaultKey);

/**
 * 
 */
UCLASS(Abstract)
class STILLHEAR_API UBindingsListWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(BlueprintAssignable)
	FOnFoundNotDefaultKey OnFoundNotDefaultKey;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ScrollBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> ListContainer;

	UPROPERTY()
	TArray<class UBindingRowWidget*> RowWidgets;
#pragma endregion

#pragma region UFUNCTIONS
	UFUNCTION()
	void FoundNotDefaultKey();
#pragma endregion 
	
#pragma region METHODS
public:
	void InitializeRows();
	TArray<class UBindingRowWidget*> GetRowWidgets() { return RowWidgets; }
	
protected:
	virtual void NativeConstruct() override;
#pragma endregion 
};
