// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/StillHearCamera.h"

#include "Character/StillHearCompanionCharacter.h"
#include "Character/StillHearPlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AStillHearCamera::AStillHearCamera()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UCameraComponent* cameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	RootComponent = cameraComponent;
}

void AStillHearCamera::SetController(AStillHearPlayerController* NewPlayerController)
{
	PlayerController = NewPlayerController;
}

void AStillHearCamera::InitializeCamera()
{

	FVector initialLocation(PlayerController->GetCharacter()->GetActorLocation().X, FloorDistanceFromTargetPoint, HeightFromFloor);
	SetActorLocation(initialLocation);
	CurrentDistanceFromTargetPoint = FloorDistanceFromTargetPoint;
	CurrentHeightFromFloor = HeightFromFloor;
}

// Called when the game starts or when spawned
void AStillHearCamera::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AStillHearCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector playerLocation = PlayerController->GetCharacter()->GetActorLocation();

	// In this section I look for the point of the screen that the camera has to look. The X coordinate of this point
	// will also be the X coordinate of the camera itself
	if (PlayerController)
		TargetFocusPoint = PlayerController->GetCharacter()->GetActorLocation();
	
	
	FVector startLocation = GetActorLocation();
	FVector desiredLocation(TargetFocusPoint.X, CurrentDistanceFromTargetPoint, CurrentHeightFromFloor);

	float interpSpeed = 1 / TimeToReachTargetLocation;
	FVector smoothedLocation = FMath::VInterpTo(startLocation, desiredLocation, DeltaTime, interpSpeed);
	SetActorLocation(smoothedLocation);

	FRotator startRotation = GetActorRotation();
	FRotator desiredRotation = UKismetMathLibrary::FindLookAtRotation(smoothedLocation, TargetFocusPoint);
	FRotator smoothRotation = FMath::RInterpTo(startRotation, desiredRotation, DeltaTime, interpSpeed);
	SetActorRotation(smoothRotation);

	
}

