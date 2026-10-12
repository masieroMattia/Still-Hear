// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "AIWormCharacter.generated.h"

UCLASS()
class STILLHEAR_API AAIWormCharacter : public AStillHearAICharacterBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USceneComponent> LookAtPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UCapsuleComponent> HeadCollider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UCapsuleComponent> BodyCollider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UCapsuleComponent> TailCollider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USceneComponent> TerrainVfxLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> HitEffectClass;

	
	UPROPERTY()
	FRotator InitialCapsuleRotation;

	// True once InitialCapsuleRotation has been captured in the head's neutral rest pose.
	// Prevents re-capturing it on a subsequent BeginPlay (level streaming re-shows the worm and
	// re-runs BeginPlay with the head already rotated), which would bake an offset into the head
	// rotation and displace every head-attached VFX.
	bool bCapsuleRotationInitialized = false;

	UPROPERTY()
	TArray<AStillHearAICharacterBase*> NearbyAICharacters;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	// Sets default values for this character's properties
	AAIWormCharacter();
#pragma endregion 

#pragma region UFUNCTION	
private:
	UFUNCTION()
	void ReportTouchEvent(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleOverlapEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void HandleOverlapExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void HandleHeadHitCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion
	
#pragma region METHODS
public:
	void SetLookAtPosLocation(const FVector& NewLookAtPosLocation) const;
	FVector GetLookAtPosLocation() const { return LookAtPos->GetComponentLocation(); }

	virtual void SetCollision(const TArray<TEnumAsByte<ECollisionChannel>>& Channels, bool bIgnore) override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void AdjustCapsuleRotation();
	void RegulateDistanceWithOtherAICharacters();
#pragma endregion 

};
