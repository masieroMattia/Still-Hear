#pragma once

#include "CoreMinimal.h"
#include "StillHearCharacterBase.h"
#include "CompanionStatus.h"
#include "StillHearCompanionCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCompanionDeath);

UCLASS()
class STILLHEAR_API AStillHearCompanionCharacter : public AStillHearCharacterBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<class UBasicAttributeSet> AttributeSet;
    UPROPERTY(BlueprintCallable, Category = "Events")
	FOnCompanionDeath OnCompanionDeathDelegate;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> JoyMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> AngerMeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> FearMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation")
	bool bRotateOnItself;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation")
	float RotationSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Color")
	FLinearColor JoyColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Color")
	FLinearColor AngerColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Color")
	FLinearColor FearColor;

	// Reference to the material in order to be able to change color at runtime
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

	UPROPERTY()
	ECompanionStatus CurrentStatus;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Floating")
	float FloatingHeight;
	UPROPERTY(EditDefaultsOnly, Category = "Floating")
	float FloatingHeightOffset;
	UPROPERTY(EditDefaultsOnly, Category = "Floating")
	float FloatingCycleTime;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowTrace;
	
	UPROPERTY(EditDefaultsOnly, Category = "InitialStats")
	TSubclassOf<UGameplayEffect> StatsInitializerGameplayEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> SoundWaveAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> DeathAbilityClass;
#pragma endregion 

#pragma region VARIABLES
private:
	float TargetFloatingHeight;
	float FloatingCycleElapsedTime;
	bool bFloat;
	float InitialLocationZ;
	float TargetLocationZ;
	bool bReadjusting;
	float FloorCheckTimer = 0.0f;
	float LastKnownFloorHeight = 0.0f;
	
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	AStillHearCompanionCharacter();
#pragma endregion 

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetStatus(const ECompanionStatus NewStatus);

	UFUNCTION(BlueprintCallable, Category = "Status")
	ECompanionStatus GetStatus() const { return CurrentStatus; }
#pragma endregion 
	
#pragma region METHODS
public:
	// Abilities Section
	void StartSoundWave() const; 
	void ShootSoundWave();
	void InterruptSoundWave();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void HandleDeath(const FGameplayTag DeathTag, int32 NewCount) override;
	
private:
	void InitializeStats() const;
	
	float CheckFloorHeight() const;
	bool IsNotInFloatingRange(const float& CurrentDistanceFromFloor) const;
	void DrawDebugRayCast(const FVector& Start, const FVector& End, bool Hit, const FHitResult& HitResult) const;
	void HandleAiming(const FGameplayTag AimTag, int32 NewCount);

	void Levitate();
	void Rotate();
	USkeletalMeshComponent* GetCurrentMesh() const;
#pragma endregion 
};
