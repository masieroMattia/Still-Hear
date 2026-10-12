// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BindingRowWidget.h"
#include "KeyboardMoveRowWidget.generated.h"

/**
 * 
 */

enum class EKeyboardMoveDirection : uint8;

UCLASS()
class STILLHEAR_API UKeyboardMoveRowWidget : public UBindingRowWidget
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration|Direction")
	EKeyboardMoveDirection Direction;
#pragma endregion

#pragma region METHODS
public:
	EKeyboardMoveDirection GetDirection() const { return Direction; }
protected:
	virtual void NativeConstruct() override;
	virtual void SetInitialGlyph() override;
#pragma endregion 
};
