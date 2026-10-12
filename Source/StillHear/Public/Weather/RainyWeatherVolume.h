// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RainyWeatherVolume.generated.h"

UCLASS()
class STILLHEAR_API ARainyWeatherVolume : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(EditAnywhere, Category = "Configuration|Data Asset")
	TObjectPtr<class URainyWeatherDataAsset> RainyWeatherDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Material Parameter Collection")
	TObjectPtr<class UMaterialParameterCollection> RainMaterialParameterCollection;
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Material Parameter Collection")
	TObjectPtr<class UMaterialParameterCollection> VolumetricCloudsMaterialParameterCollection;

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Texture")
	TArray<TObjectPtr<class UTexture2D>> LightningTextures;

	UPROPERTY()
	TObjectPtr<class UMaterialParameterCollectionInstance> RainMaterialParameterCollectionInstance;
	UPROPERTY()
	TObjectPtr<class UMaterialParameterCollectionInstance> VolumetricCloudsMaterialParameterCollectionInstance;
	
	UPROPERTY(VisibleAnywhere, Category = "Root")
	TObjectPtr<class UBoxComponent> Volume;

	UPROPERTY(EditAnywhere, Category = "PostProcess")
	TObjectPtr<class UPostProcessComponent> PostProcess;

	UPROPERTY(EditDefaultsOnly, Category = "Rain")
	TObjectPtr<class UNiagaraComponent> RainVFX;

	UPROPERTY(VisibleAnywhere, Category = "Lightning")
	TObjectPtr<class USceneComponent> LightningArea;
	
	UPROPERTY(VisibleAnywhere, Category = "Lightning")
	TObjectPtr<class UStaticMeshComponent> LightningPlane;

	UPROPERTY()
	TObjectPtr<class ACharacter> CharacterInVolume;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> LightningMaterialInstance;
#pragma endregion

#pragma region VARIABLES
private:
	FTimerHandle WindTimerHandle;
	FTimerHandle RainTimerHandle;
	FTimerHandle LightningStartTimerHandle;
	FTimerHandle LightningCycleTimerHandle;
	FTimerHandle VolumetricCloudsTimerHandle;
	
	// Follow Player
	bool bRainFollowPlayer = false;
	bool bLightningFollowPlayer = false;

	// Activate or Deactivate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (AllowPrivateAccess = "true"))
	bool bCinematicMode = false;

	bool bActivate = false;
	bool bDeactivate = true;
	
	// Rain Lerping
	float TrueMaxSpawnRate;
	float LerpingRainTime = 0;
	bool bLerpingRain = false;

	// Wind Lerping
	float LerpingWindTime = 0;
	bool bLerpingWind = false;

	// Lightning Curve
	bool bTriggerLightning = false;
	float CurveLightningTime;
	float CurrentLightningTime;

	int LightningTexturesAmount;

	// PostProcess Exposure Value Cache
	float InitialPostProcessExposure;

	// Volumetric Clouds Unreal Default Values
	float StormClouds;
	float Storm_LightningTexScale;
	float LightningFlicker;
	float DynamicLightingAnim;
	float ManualLightningAnim;
	float SourcePower;
	float FillScatter;
	float FillScatterIntensity;
	float SecondMipLevel;
	float LightningMaskBias;
	float LightningMaskStrength;
	float CloudTextureWeight;
	FLinearColor Storm_LightningColor;
	FLinearColor Storm_AlbedoColor;

	// Volumetric Cloud Lerping
	float VolumetricCloudTime = 0;
	bool bLerpingClouds = false;

#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	ARainyWeatherVolume();
#pragma endregion

#pragma region UFUNCTION
private:
	UFUNCTION()
	void Activate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void Deactivate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion 
	
#pragma region METHODS
private:
	void MoveLightnings() const;
	void MoveRain() const;
	
	void StartRain();
	void LerpRain(const float DeltaTime);
	void StopRain();
	
	void StartWind();
	void LerpWind(const float DeltaTime);
	void StopWind();
	
	void TriggerLightning();
	void GoThroughLightningCurve(const float DeltaTime);
	void StopLightnings();
	void SetNextLightningTime();	

	void ChangeVolumetricClouds();
	void LerpClouds(const float DeltaTime);
	void ResetVolumetricClouds();

#if WITH_EDITOR
	void DrawDebugThunderArea();
	void DrawDebugRainArea();
#endif
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
	
public:
	virtual void Tick(float DeltaTime) override;
#pragma endregion 
};
