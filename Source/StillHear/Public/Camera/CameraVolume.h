// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraVolume.generated.h"

UCLASS(Abstract)
class STILLHEAR_API ACameraVolume : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UCameraComponent> CameraComponent;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> Volume;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> OuterVolume;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UArrowComponent> InputArrow;

	UPROPERTY(VisibleAnywhere, Category = "ActorInside")
	AActor* ActorInVolume;

	UPROPERTY()
	FTransform DefaultCameraTransform;

	// When true, the next UpdateCamera call will snap the camera to target instantly (no interpolation)
	bool bShouldSnapToTarget = false;
	
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Linear;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	float BlendExp = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	float BlendTimeOnEnter;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	bool bUseBlendParametersOnExit;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraBlend")
	float BlendTimeOnExit;
	UPROPERTY(EditAnywhere, Category = "Configuration")
	int Priority;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraRotation")
	bool bLookAtPlayer;
	UPROPERTY(EditAnywhere, Category = "Configuration|CameraRotation", meta = (EditCondition = "bLookAtPlayer", EditConditionHides))
	float RotationSpeed;
	UPROPERTY(EditAnywhere, Category = "Configuration|Input")
	bool bInputFollowsCamera;
	UPROPERTY(EditAnywhere, Category = "Configuration|Input")
	float InputAdjustingTime = 2.0f;

	bool bHasPlayerAdjustedToInput = false;

protected:
	ACameraVolume();
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Camera")
	UCameraComponent* GetCamera() const;

	// Returns true if WorldLocation is strictly inside the inner Volume box (accounts for rotation)
	bool ContainsPoint(const FVector& WorldLocation) const;

	// Utility functions to symbolize this Camera Volume is currently active and using its camera to follow the character
	UFUNCTION(BlueprintCallable, Category = "Init")
	void Activate(AActor* Actor);
	UFUNCTION(BlueprintCallable, Category = "Init")
	void Deactivate();

	// Request the camera to snap to target position on the next update (no interpolation)
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RequestSnapToTarget();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetToDefaultTransform();
	
	// Returns true if the camera is pending a snap (no blend)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Camera")
	bool IsSnappingToTarget() const { return bShouldSnapToTarget; }

	// Utility functions to retrieve the blending parameters
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlendTime")
	float GetBlendTimeOnEnter() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlendTime")
	bool GetUseBlendParametersOnExit() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BlendTime")
	float GetBlendTimeOnExit() const;
	TEnumAsByte<EViewTargetBlendFunction> GetBlendFunction() const;
	float GetBlendExp() const;

	// Input Direction Retrieving Function
	UFUNCTION(BlueprintCallable, Category = "Input")
	FVector GetRightDirection() const;
	UFUNCTION(BlueprintCallable, Category = "Input")
	FORCEINLINE float GetInputAdjustingTime() const { return InputAdjustingTime; }
	UFUNCTION(BlueprintCallable, Category = "Input")
	FORCEINLINE bool GetInputFollowsCamera() const { return bInputFollowsCamera; }
	UFUNCTION(BlueprintCallable, Category = "Input")
	FORCEINLINE bool GetHasPlayerAdjustedToInput() const { return bHasPlayerAdjustedToInput; }
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetHasPlayerAdjustedToInput(const bool bAdjusted) { bHasPlayerAdjustedToInput = bAdjusted; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Priority")
	int GetPriority() const;

	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void UpdateCamera(FVector TargetPoint, float DeltaTime);

private:
	void UpdateInputArrow();
};
