// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "StillHearControllerData.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UStillHearControllerData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gamepad")
	FName GamepadNameOverride = FName("");
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UStillHearControllerData();
#pragma endregion

#pragma region METHODS
public:
	void UpdateGamepadName();
#pragma endregion
};
