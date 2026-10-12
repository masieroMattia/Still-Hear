// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraVolumesInteractor.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCameraVolumesInteractor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STILLHEAR_API ICameraVolumesInteractor
{
	GENERATED_BODY()

#pragma region UFUNCTIONS
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	void AddCameraVolumeToList(ACameraVolume* CameraVolume);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	void RemoveCameraVolumeFromList(ACameraVolume* CameraVolume);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	FVector GetTargetPointLocation();
#pragma endregion 
};
