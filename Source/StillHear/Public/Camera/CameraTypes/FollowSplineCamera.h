// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraVolume.h"
#include "FollowSplineCamera.generated.h"

UCLASS()
class STILLHEAR_API AFollowSplineCamera : public ACameraVolume
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class USplineComponent> Spline;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	float TimeToReachTargetPoint;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraOffset")
	float OffsetAlongSpline;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraOffset")
	FVector LocationOffset;

	UPROPERTY()
	FVector DesiredCameraLocation;
public:
	AFollowSplineCamera();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void UpdateCamera(FVector TargetPoint, float DeltaTime) override;

private:
	void ApplyPosition(float DeltaTime);
};
