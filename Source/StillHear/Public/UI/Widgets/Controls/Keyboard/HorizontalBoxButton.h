// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Elements/ButtonBase.h"
#include "HorizontalBoxButton.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UHorizontalBoxButton : public UButtonBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> HorizontalBox;
};
