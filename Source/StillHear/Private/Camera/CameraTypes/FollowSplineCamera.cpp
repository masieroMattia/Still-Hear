// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CameraTypes/FollowSplineCamera.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"


AFollowSplineCamera::AFollowSplineCamera()
{
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Volume);
}

void AFollowSplineCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyPosition(DeltaSeconds);
}


void AFollowSplineCamera::UpdateCamera(FVector TargetPoint, float DeltaTime)
{
	Super::UpdateCamera(TargetPoint, DeltaTime);

	// Finding the percentage of the spline closest to the target point
	const float InputKey = Spline->FindInputKeyClosestToWorldLocation(TargetPoint);
	// Finding the distance along the spline of the input key
	float Distance = Spline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	Distance += OffsetAlongSpline;
	Distance = FMath::Clamp(Distance, 0.0f, Spline->GetSplineLength());
	
	// Finding the point of the spline closest to the percentage that we found
	DesiredCameraLocation = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) + LocationOffset;

	/*
	if (bShouldSnapToTarget)
	{
		// Snap instantly to the desired location (no interpolation)
		CameraComponent->SetWorldLocation(DesiredCameraLocation);
		bShouldSnapToTarget = false;
	}
	else
	{
		// Calculating movement of the camera along the spline
		FVector CurrentCameraLocation = CameraComponent->GetComponentLocation();
		float InterpSpeed = (TimeToReachTargetPoint > KINDA_SMALL_NUMBER) ? (1.0f / TimeToReachTargetPoint) : 0.0f;
		FVector SmoothLocation = FMath::VInterpTo(CurrentCameraLocation, DesiredCameraLocation, DeltaTime, InterpSpeed);
		CameraComponent->SetWorldLocation(SmoothLocation);
	}
	*/
}

void AFollowSplineCamera::ApplyPosition(float DeltaTime)
{
	if (bShouldSnapToTarget)
	{
		// Snap instantly to the desired location (no interpolation)
		CameraComponent->SetWorldLocation(DesiredCameraLocation);
		bShouldSnapToTarget = false;
	}
	else
	{
		// Calculating movement of the camera along the spline
		FVector CurrentCameraLocation = CameraComponent->GetComponentLocation();
		float InterpSpeed = (TimeToReachTargetPoint > KINDA_SMALL_NUMBER) ? (1.0f / TimeToReachTargetPoint) : 0.0f;
		FVector SmoothLocation = FMath::VInterpTo(CurrentCameraLocation, DesiredCameraLocation, DeltaTime, InterpSpeed);
		CameraComponent->SetWorldLocation(SmoothLocation);
	}
}
