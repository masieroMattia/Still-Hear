// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FollowSplineCamera.h"
#include "FollowSplineMaintainDistanceCamera.generated.h"

UCLASS()
class STILLHEAR_API AFollowSplineMaintainDistanceCamera : public AFollowSplineCamera
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditAnywhere, Category = "Configuration|Distance From Actor")
	float DistanceFromActor;
	UPROPERTY(EditAnywhere, Category = "Configuration|Distance From Actor")
	float MaxHeight;
	UPROPERTY(EditAnywhere, Category = "Configuration|Distance From Actor")
	float MinHeight;
	UPROPERTY(EditAnywhere, Category = "Configuration|Distance From Actor")
	bool ShowDesiredLocationHeight;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	// Sets default values for this actor's properties
	AFollowSplineMaintainDistanceCamera();
#pragma endregion 

#pragma region METHODS
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void UpdateCamera(FVector TargetPoint, float DeltaTime) override;
#pragma endregion 
};
