#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

UCLASS()
class STILLHEAR_API AProjectile : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProjectileNiagaraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ImpactNiagaraComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Projectile Settings|Config")
	float BaseSpeed = 200.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Projectile Settings|Config")
	float DeflectedSpeed = 1000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "ProjectileSettings|Config")
	float LifeSpan = 10.0f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Projectile Settings|Deflection")
	float DeflectionPauseDuration = 4.0f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Projectile SettingS|Sounds")
	float BasePitchMultiplier = 0.1f;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Projectile SettingS|Sounds")
	float OffsetPitchMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Sounds")
	TObjectPtr<USoundBase> ProjectileTrailSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Sounds")
	TObjectPtr<USoundBase> ProjectileHitSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Sounds")
	TObjectPtr<USoundBase> DeflectEnterSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Sounds")
	TObjectPtr<USoundBase> DeflectExitSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Sounds")
	TObjectPtr<USoundBase> EnemyHitSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|VFX")
	TObjectPtr<UNiagaraSystem> EnemyHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Noise")
	float NoiseLoudness = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Noise")
	float NoiseRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings|Noise")
	FVector OriginLocation = FVector::ZeroVector;
#pragma endregion
	
#pragma region VARIABLES
protected:
	int32 DeflectionCount = 0; // Number of times the projectile has been deflected
	bool bIsPaused = false;
	
	float CurrentSpeed = 0.0f; // Tracks the current speed of the projectile
	
	FVector NextForwardVector;
	FVector DeflectionDirection;
	
	float DeflectionDistanceTime = 0.0f;
	float TimeToExit = 0.0f;
	
	FTimerHandle MoveToSpotTimerHandle;
	FTimerHandle PauseTimerHandle;
	FTimerHandle EnableCollisionTimerHandle;
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentDeflector;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitDeflectors;
	UPROPERTY()
	TObjectPtr<UAudioComponent> TrailAudioComponent;
	UPROPERTY()
	TObjectPtr<UAudioComponent> DeflectEnterSoundSpawned;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AProjectile();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void LifeSpanExpired() override;
	
protected:
	virtual void BeginPlay() override;
	
public:
	void SetHomingTarget(USceneComponent* TargetComponent, float InHomingAcceleration) const;
	
private:
	void StartMovingToSpot();
	void PauseAtSpot();
	void ResumeMovement();
	void EnableCollision();
	void StopAudioComponentAndDestroyActor(TObjectPtr<UAudioComponent> AudioComponent);
#pragma endregion
	
#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
#pragma endregion
};