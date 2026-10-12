// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StillHearControllerData.h"

UStillHearControllerData::UStillHearControllerData()
{
	GamepadName = GamepadNameOverride;
}

void UStillHearControllerData::UpdateGamepadName()
{
	GamepadName = GamepadNameOverride;
}
