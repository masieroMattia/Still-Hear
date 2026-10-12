#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraVolume.h"
#include "StillHearCharacterBase.h"
#include "Perception/AISense_Sight.h"
#include "Components/SphereComponent.h"
#include "Perception/AISightTargetInterface.h"
#include "Interfaces/CameraVolumesInteractor.h"
#include "Character/MainCharacterAbilityType.h"
#include "StillHearMainCharacter.generated.h"

class UFloatingCompanionComponent;
class UResonanceManagerComponent;
class AStillHearPlayerController;
class UMotionWarpingComponent;
class UForceFeedbackEffect;
class UBlobShadowComponent;
class UNiagaraComponent;
class UFootStepData;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInizializedFinished);

UCLASS()
class STILLHEAR_API AStillHearMainCharacter : public AStillHearCharacterBase, public ICameraVolumesInteractor, public IAISightTargetInterface
{
	GENERATED_BODY()

	friend class AStillHearPlayerController;

#pragma region UPROPERTIES

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<class UMainCharacterAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UResonanceManagerComponent> ResonanceManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFloatingCompanionComponent> CompanionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> ParrySphereComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlobShadowComponent> BlobShadow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterStatistics")
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterStatistics")
	float CrouchedSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterStatistics")
	float CrouchedHeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterStatistics")
	float DeathHeightThreshold;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CharacterStatistics")
	float CoyoteTimeForJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sockets")
	FName LeftHandSocketName = FName("LeftHand_Socket");
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sockets")
	FName RightHandSocketName = FName("RightHand_Socket");

private:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> ParryAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> ResonanceAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> InteractionAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> SprintAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> CrouchAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> JumpAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> LowVaultAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> ClimbAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> DeathAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category = "GameplayAbility")
	TSubclassOf<UGameplayAbility> SoundWaveAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "InitialStats")
	TSubclassOf<UGameplayEffect> StatsInitializerGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UBlendSpace> DefaultBlendSpace;

	// If true, the character sits down automatically after IdleTimeBeforeSit seconds of inactivity
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Sitting")
	bool bEnableIdleSit = true;
	// Montage played when the character sits down due to inactivity
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Sitting", meta = (EditCondition = "bEnableIdleSit", EditConditionHides))
	TObjectPtr<UAnimMontage> SitDownMontage;
	// Seconds of inactivity before the character sits down
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Sitting", meta = (EditCondition = "bEnableIdleSit", EditConditionHides, ClampMin = "0.1"))
	float IdleTimeBeforeSit = 20.0f;
	// If true, the character starts the level seated; stands up on the first player input
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Sitting")
	bool bStartSittingOnSpawn = false;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Camera")
	TArray<ACameraVolume*> CameraVolumesList;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Camera")
	TObjectPtr<ACameraVolume> LastActiveCameraVolume;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<UFootStepData> FootstepConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Feedback")
	TObjectPtr<UForceFeedbackEffect> LandForceFeedback;


public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInizializedFinished OnInitializedFinished;
#pragma endregion

#pragma region VARIABLES

private:
	// References passed from player controller, useful for vaulting and climbing.
	// Gameplay Abilities will read from here
	FVector WallNormalReferenceForClimbing;
	FVector WallHeightReferenceForClimbing;
	FVector MotionWarpTarget;

	float StartFallingHeight;
	bool bHasJumped = false;

	FTimerHandle CoyoteTimerHandle;
	bool bPersonalizedCanJump;

	// When true, the next camera activation will snap instantly (no
	// interpolation)
	bool bForceSnapOnNextCamera = false;

	// Cached default mesh transform for revive (saved at BeginPlay)
	FVector DefaultMeshRelativeLocation;
	FRotator DefaultMeshRelativeRotation;

	// True while the character is sitting down/seated due to player inactivity
	bool bIsSitting = false;

	UPROPERTY()
	TObjectPtr<AStillHearPlayerController> PCRef;

	// Cached anim instance
	UPROPERTY()
	TObjectPtr<class UMainCharacterAnimInstance> MainAnimInstance;
#pragma endregion

#pragma region CONSTRUCTOR

public:
	AStillHearMainCharacter();
#pragma endregion

#pragma region METHODS

public:
	// Abilities Section
	void Sprint() const;
	void StopSprinting();
	void CrouchAbility();
	void StartCrouch() const;
	void ReleaseCrouch();
	void JumpAbility() const;
	// void LowVault(const FVector& WallHeight, const FVector& LandingPoint);
	// void LowGetOnTop() const;
	void Climb(const FVector& ImpactPoint);

	void Parry() const;
	void ActivateResonance() const;
	void DeactivateResonance();
	void ActivateResonanceInteraction();
	void TapInteraction();

	// Companion Abilities Section
	void StartSoundWave() const;
	void ShootSoundWave();
	void InterruptSoundWave() const;

	// Utility function for jumping coyote time
	bool PersonalizedCanJump() const;
	bool IsJumping() const;

	// Helper to get ability class by enum type
	TSubclassOf<UGameplayAbility>
	GetAbilityClassByType(EMainCharacterAbilityType AbilityType) const;

	// Getter for attribute set, useful to make some checks of abilities'
	// usability

	UMainCharacterAttributeSet* GetAttributeSet();
	UResonanceManagerComponent* GetResonanceManagerComponent() const { return ResonanceManagerComponent; }
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarping; }
	USphereComponent* GetParrySphere() const { return ParrySphereComponent; }

	// Animation Functions
	void SetDefaultLocomotionBlendSpace() const;
	void SetLocomotionBlendSpace(UBlendSpace* BlendSpaceToApply) const;
	void SetAnimationClimbing(bool Climbing) const;
	FVector GetWallNormalReference() const;
	FVector GetWallHeightReference() const;
	FVector GetMotionWarpTarget() const;
	void SetMotionWarpTarget(const FVector& MotionTargetReference);

	// Camera Volume functions
	virtual FVector GetTargetPointLocation_Implementation() override;
	virtual void AddCameraVolumeToList_Implementation(ACameraVolume* CameraVolume) override;
	virtual void RemoveCameraVolumeFromList_Implementation(ACameraVolume* CameraVolume) override;
	ACameraVolume* GetLastActiveCameraVolume() const;
	ACameraVolume* GetHighestPriorityCameraVolume() const;
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetLastActiveCameraVolume() { LastActiveCameraVolume = nullptr; }
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetForceSnapOnNextCamera(bool bForceSnap) { bForceSnapOnNextCamera = bForceSnap; }

	// AI Sight Interface
	virtual UAISense_Sight::EVisibilityResult
	CanBeSeenFrom(const FCanBeSeenFromContext& Context, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested, float& OutSightStrength, int32* UserData = nullptr, const FOnPendingVisibilityQueryProcessedDelegate* Delegate = nullptr) override;

	// Floor detection
	EPhysicalSurface DetectFloorType() const;
	FORCEINLINE UFootStepData* GetFootstepConfig() const { return FootstepConfig; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	// Gameplay Tag handlers
	virtual void HandleForceMoving(const FGameplayTag ForceMovingTag, int32 NewCount);
	virtual void HandleDragging(const FGameplayTag FreeDraggingTag, int32 NewCount);
	virtual void HandleAttackHit(const FGameplayTag AttackHitTag, int32 NewCount);
	virtual void HandleDeath(const FGameplayTag DeathTag, int32 NewCount) override;

public:
	// Revive the character: undo ragdoll, re-enable movement and collisions
	void Revive();
	// Refresh permanently unlocked abilities from SaveSubsystem
	void RefreshPermanentAbilities();

private:
	void InitializeStats() const;
	void StartCoyoteTime();
	void ResetPersonalizedCanJump();

	void StandUp();

public:
	// Plays SitDownMontage and locks movement, used when the player has been idle for too long
	void SitDown();
	FORCEINLINE bool IsSitting() const { return bIsSitting; }
	FORCEINLINE bool IsIdleSitEnabled() const { return bEnableIdleSit; }
	FORCEINLINE float GetIdleTimeBeforeSit() const { return IdleTimeBeforeSit; }
	// True while any granted gameplay ability is currently active
	bool IsPerformingAbility() const;

private:
	void OnSitDownBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	// Handling Camera Volumes
	void ActivateCameraVolume(ACameraVolume* CameraVolume);
	virtual void CheckList();
	virtual void CheckFirstCameraAtSpawn();
#pragma endregion
};
