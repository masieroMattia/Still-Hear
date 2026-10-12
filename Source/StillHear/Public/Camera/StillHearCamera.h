// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "StillHearCamera.generated.h"

UCLASS()
class STILLHEAR_API AStillHearCamera : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float FloorDistanceFromTargetPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float HeightFromFloor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float TimeToReachTargetLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float SpeedToGoAway;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float SpeedToComeBack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings")
	float ToleranceForScreenBounds;

private:
	FVector TargetFocusPoint;
	float CurrentDistanceFromTargetPoint;
	float CurrentHeightFromFloor;

	UPROPERTY(VisibleAnywhere, Category = "CameraSettings")
	TObjectPtr<class AStillHearPlayerController> PlayerController;
	
public:
	// Sets default values for this actor's properties
	AStillHearCamera();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetController(AStillHearPlayerController* NewPlayerController);
	void InitializeCamera();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
