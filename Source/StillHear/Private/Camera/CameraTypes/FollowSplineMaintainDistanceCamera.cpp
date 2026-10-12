// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CameraTypes/FollowSplineMaintainDistanceCamera.h"

#include "Camera/CameraComponent.h"


// Sets default values
AFollowSplineMaintainDistanceCamera::AFollowSplineMaintainDistanceCamera()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFollowSplineMaintainDistanceCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFollowSplineMaintainDistanceCamera::UpdateCamera(FVector TargetPoint, float DeltaTime)
{
	Super::UpdateCamera(TargetPoint, DeltaTime);

	// Direction
	FVector Direction = DesiredCameraLocation - TargetPoint;
	Direction.Normalize();

	// Target Position from the player/target point
	DesiredCameraLocation = TargetPoint + Direction * DistanceFromActor;

	
	if (MaxHeight != 0 && DesiredCameraLocation.Z > MaxHeight)
		DesiredCameraLocation.Z = MaxHeight;

	if (MinHeight != 0 && DesiredCameraLocation.Z < MinHeight)
		DesiredCameraLocation.Z = MinHeight;

#if WITH_EDITOR
	// In order to decide and adjust the height properly I strongly suggest to analyze the height case by case by displaying this value first
	if (ShowDesiredLocationHeight && GEngine)
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::FromInt(DesiredCameraLocation.Z));
#endif
	
}

