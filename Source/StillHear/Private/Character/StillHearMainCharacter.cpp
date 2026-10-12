#include "Character/StillHearMainCharacter.h"

#include "NativeGameplayTags.h"
#include "Animation/AnimMontage.h"
#include "StillHearGameInstance.h"
#include "MotionWarpingComponent.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Perception/AISense_Hearing.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "VFX/ResonanceManagerComponent.h"
#include "TraceAndCollision/CustomSurface.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "Character/StillHearPlayerController.h"
#include "Character/FloatingCompanionComponent.h"
#include "Character/StillHearCompanionCharacter.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Character/Components/BlobShadowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Animation/AnimInstances/MainCharacterAnimInstance.h"
#include "GameplayAbilitySystem/Attributes/MainCharacterAttributeSet.h"

// Sets default values
AStillHearMainCharacter::AStillHearMainCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to
	// improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add the Attribute Set
	AttributeSet = CreateDefaultSubobject<UMainCharacterAttributeSet>(TEXT("AttributeSet"));

	ResonanceManagerComponent = CreateDefaultSubobject<UResonanceManagerComponent>(TEXT("ResonanceManager"));
	ResonanceManagerComponent->bAutoActivate = false;

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	CompanionComponent = CreateDefaultSubobject<UFloatingCompanionComponent>(TEXT("CompanionComponent"));

	// Add the Parry Sphere Component
	ParrySphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ParrySphereComponent"));
	ParrySphereComponent->SetupAttachment(RootComponent);
	ParrySphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ParrySphereComponent->ComponentTags.Add(TAG_ParrySphere.GetTag().GetTagName());

	BlobShadow = CreateDefaultSubobject<UBlobShadowComponent>(TEXT("BlobShadow"));
	BlobShadow->SetupAttachment(RootComponent);
}

void AStillHearMainCharacter::RemoveCameraVolumeFromList_Implementation(
	ACameraVolume* CameraVolume)
{
	// I'm sure CameraVolumesList contains CameraVolume because I can never call
	// this function if I didn't call the Add function first.
	CameraVolumesList.Remove(CameraVolume);

	if (CameraVolumesList.Num() == 0)
		return;

	CheckList();
}

ACameraVolume* AStillHearMainCharacter::GetLastActiveCameraVolume() const
{
	return LastActiveCameraVolume;
}

ACameraVolume* AStillHearMainCharacter::GetHighestPriorityCameraVolume() const
{
	if (CameraVolumesList.Num() == 0)
		return nullptr;

	int32 MaxPriority = -1;
	ACameraVolume* BestVolume = nullptr;
	for (ACameraVolume* Volume : CameraVolumesList)
	{
		if (IsValid(Volume) && Volume->GetPriority() > MaxPriority)
		{
			MaxPriority = Volume->GetPriority();
			BestVolume = Volume;
		}
	}
	return BestVolume;
}

EPhysicalSurface AStillHearMainCharacter::DetectFloorType() const
{
	// Line trace to find the surface type
	FHitResult HitResult;
	const FVector Start = GetMesh()->GetComponentLocation();
	const FVector End = Start - FVector(0, 0, 500); // Trace down
	FCollisionQueryParams Params;
	Params.AddIgnoredComponent(GetMesh());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (HasAnyFlags(RF_ClassDefaultObject))
		return SurfaceType_Default;

	if (!bHit)
		return SurfaceType_Default;

	// Robust retrieval of physical material: try HitResult.PhysMaterial,
	// BodyInstance override, material physical material
	const UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.IsValid() ? HitResult.PhysMaterial.Get() : nullptr;

	if (!PhysMat && HitResult.Component.IsValid())
	{
		const UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HitResult.Component.Get());
		if (PrimComp)
		{
			// Try BodyInstance simple phys material (override)
			if (GEngine && PrimComp->GetBodyInstance())
			{
				PhysMat = PrimComp->GetBodyInstance()->GetSimplePhysicalMaterial();
			}

			// Fallback: try material(0) physical material
			if (!PhysMat)
			{
				const int32 MaterialIndex = HitResult.FaceIndex >= 0 ? HitResult.FaceIndex : 0;
				const UMaterialInterface* Mat = PrimComp->GetMaterial(MaterialIndex);
				if (Mat)
				{
					PhysMat = Mat->GetPhysicalMaterial();
				}
			}
		}
	}

	return PhysMat ? UPhysicalMaterial::DetermineSurfaceType(PhysMat) : SurfaceType_Default;
}

// Called when the game starts or when spawned
void AStillHearMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_MainCharacter_ForceMoving).AddUObject(this, &ThisClass::HandleForceMoving);
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_FreeDragging).AddUObject(this, &ThisClass::HandleDragging);
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_RailDragging).AddUObject(this, &ThisClass::HandleDragging);
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Event_AttackHit).AddUObject(this, &ThisClass::HandleAttackHit);
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_Death).AddUObject(this, &ThisClass::HandleDeath);

	// Cache the default mesh transform for revive
	DefaultMeshRelativeLocation = GetMesh()->GetRelativeLocation();
	DefaultMeshRelativeRotation = GetMesh()->GetRelativeRotation();

	// Cache the anim instance
	MainAnimInstance = Cast<UMainCharacterAnimInstance>(GetMesh()->GetAnimInstance());

	InitializeStats();
	bForceSnapOnNextCamera = true;
	FTimerHandle CameraTimerHandle;
	GetWorldTimerManager().SetTimer(CameraTimerHandle, this, &AStillHearMainCharacter::CheckFirstCameraAtSpawn, 0.05f, false);
	FTimerHandle InitTimerHandle;
	GetWorldTimerManager().SetTimer(InitTimerHandle, [this]()
	{
		OnInitializedFinished.Broadcast();
	}, 0.5f, false);

	bool bIsNewGame = false;
	if (const UStillHearGameInstance* GI = GetGameInstance<UStillHearGameInstance>())
	{
		bIsNewGame = GI->bIsSpawningNewGame;
	}

	if (bStartSittingOnSpawn && !bIsNewGame && MainAnimInstance)
	{
		// Start seated, the player will stand up on the first Move input
		bIsSitting = true;
		MainAnimInstance->bIsSitting = true;
	}
}


void AStillHearMainCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PCRef = Cast<AStillHearPlayerController>(GetController());

	if (PCRef)
	{
		CheckList();

	}

	// Grant permanently unlocked abilities from SaveSystem
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USaveSubsystem* SaveSS = GI->GetSubsystem<USaveSubsystem>())
		{
			TSet<EMainCharacterAbilityType> Unlocked = SaveSS->GetPermanentlyUnlockedAbilities();
			TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

			for (const EMainCharacterAbilityType AbilityType : Unlocked)
			{
				if (TSubclassOf<UGameplayAbility> AbilityClass = GetAbilityClassByType(AbilityType))
				{
					AbilitiesToGrant.Add(AbilityClass);
				}
			}

			if (AbilitiesToGrant.Num() > 0)
			{
				GrantAbilities(AbilitiesToGrant);
			}
		}
	}
}

void AStillHearMainCharacter::InitializeStats() const
{
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedSpeed;
	GetCharacterMovement()->SetCrouchedHalfHeight(CrouchedHeight / 2);

	if (!AbilitySystemComponent)
		return;

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StatsInitializerGameplayEffectClass, 1.0f, AbilitySystemComponent->MakeEffectContext());

	if (!SpecHandle.IsValid())
		return;

	// Initialize all the stats here by setting their attribute value through the
	// "value initializer" gameplay effect
	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.BaseSpeed")), BaseSpeed);

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")), 1.0f);

	// Apply the effect to change the values in the character class
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void AStillHearMainCharacter::StartCoyoteTime()
{
	GetWorldTimerManager().SetTimer(
		CoyoteTimerHandle,
		this,
		&ThisClass::ResetPersonalizedCanJump,
		CoyoteTimeForJump,
		false
	);
}

void AStillHearMainCharacter::ResetPersonalizedCanJump()
{
	bPersonalizedCanJump = false;
}

// If I call this function, it means that the current volume I'm in is different
// from LastVisitedCameraVolume This means LastVisitedCameraVolume could be
// nullptr or just a different volume
void AStillHearMainCharacter::ActivateCameraVolume(ACameraVolume* CameraVolume)
{
	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetController());
	if (!PC)
		return;

	// If we need to snap (spawn/respawn), request it before changing camera
	if (bForceSnapOnNextCamera)
	{
		CameraVolume->RequestSnapToTarget();
		bForceSnapOnNextCamera = false;
	}

	CameraVolume->Activate(this);
	PC->ChangeCamera(CameraVolume, LastActiveCameraVolume);

	// I deactivate the last camera volume I visited in order to have less tick
	// operations
	if (LastActiveCameraVolume && LastActiveCameraVolume != CameraVolume)
		LastActiveCameraVolume->Deactivate();

	LastActiveCameraVolume = CameraVolume;
}

// This function is called every time the character enters or exits from a
// volume
void AStillHearMainCharacter::CheckList()
{
	// If I'm inside this function it means the number of volumes in the list
	// cannot be 0. If it is, I shouldn't call this function.
	if (CameraVolumesList.Num() == 1)
	{
		if (!IsValid(CameraVolumesList[0]) || CameraVolumesList[0] == LastActiveCameraVolume)
			return;

		// Tell the Player Controller to follow this camera since it's the only one
		ActivateCameraVolume(CameraVolumesList[0]);
		
		return;
	}

	// I have more than one volume in the list, so I have to decide which one to
	// take In order to do that, I select the volume with HIGHEST priority
	int MaxPriority = -1;
	ACameraVolume* NewCameraVolume = nullptr;
	for (const auto Volume : CameraVolumesList)
	{
		if (!IsValid(Volume))
			continue;

		if (Volume->GetPriority() > MaxPriority)
		{
			MaxPriority = Volume->GetPriority();
			NewCameraVolume = Volume;
		}
	}

	if (NewCameraVolume && NewCameraVolume != LastActiveCameraVolume)
		ActivateCameraVolume(NewCameraVolume);
}

void AStillHearMainCharacter::CheckFirstCameraAtSpawn()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->UpdateOverlaps();
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACameraVolume::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		ACameraVolume* CameraVolume = Cast<ACameraVolume>(Actor);
		if (CameraVolume)
		{
			// CameraVolume->SetSnappingToTarget(true);
			AddCameraVolumeToList_Implementation(CameraVolume);
		}
	}

	bForceSnapOnNextCamera = true;
	LastActiveCameraVolume = nullptr;
	CheckList();
}

void AStillHearMainCharacter::Falling()
{
	Super::Falling();

	StartFallingHeight = GetActorLocation().Z;

	if (!IsJumping())
	{
		bPersonalizedCanJump = true;
		StartCoyoteTime();
	}
}

void AStillHearMainCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	const float FallingDistance = StartFallingHeight - GetActorLocation().Z;

	// Use our cached Player Controller reference (PCRef) for safe and fast operations
	if (IsValid(PCRef))
	{
		PCRef->StopEdgeGrabTimer();

		// Use bHasJumped to guarantee playing the vibration after a jump
		const float EffectiveDistance = bHasJumped ? FMath::Max(FallingDistance, 150.0f) : FallingDistance;

		// Play landing force feedback when touching the ground after a notable fall/jump
		if (LandForceFeedback && EffectiveDistance > 50.0f)
		{
			FForceFeedbackParameters Params;
			Params.bLooping = false;
			Params.bIgnoreTimeDilation = true;
			PCRef->ClientPlayForceFeedback(LandForceFeedback, Params);
		}
	}

	// Reset jump flag
	bHasJumped = false;

	if (FallingDistance > DeathHeightThreshold)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DeathAbilityClass);
	}

	const EPhysicalSurface SurfaceType = DetectFloorType();

	FName Tag = FName("Run");

	if (SurfaceType == ECustomSurface::Soil)
	{
		Tag = FName(*(FName("Vibration.").ToString()) + Tag.ToString());
	}

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1, this, 0, Tag);
}

bool AStillHearMainCharacter::CanJumpInternal_Implementation() const
{
	if (Super::CanJumpInternal_Implementation())
	{
		return true;
	}

	if (bPersonalizedCanJump)
	{
		const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		return MoveComp && MoveComp->IsJumpAllowed();
	}

	return false;
}

void AStillHearMainCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	bHasJumped = true;
	bPersonalizedCanJump = false;
	GetWorldTimerManager().ClearTimer(CoyoteTimerHandle);
}

void AStillHearMainCharacter::HandleForceMoving(const FGameplayTag ForceMovingTag, const int32 NewCount)
{
	if (!IsValid(PCRef))
		return;

	if (NewCount <= 0)
		PCRef->SetForcedMoved(false);
	else
		PCRef->SetForcedMoved(true);
}

void AStillHearMainCharacter::HandleDragging(const FGameplayTag FreeDraggingTag, const int32 NewCount)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
		return;

	if (NewCount <= 0) // if NewCount is greater than 0, it means the tag was added
	{
		MovementComponent->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;

		if (!AbilitySystemComponent)
			return;

		const FGameplayTag EventTag = TAG_Event_StopInteraction;
		SendGameplayEventToSelf(EventTag);
	}
	else
	{
		MovementComponent->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
	}
}

void AStillHearMainCharacter::HandleAttackHit(const FGameplayTag AttackHitTag, int32 NewCount)
{
	if (!AbilitySystemComponent)
		return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(AttackHitTag);

	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(TagContainer);
	AbilitySystemComponent->TryActivateAbilityByClass(DeathAbilityClass);
}

void AStillHearMainCharacter::HandleDeath(const FGameplayTag DeathTag, const int32 NewCount)
{
	if (NewCount <= 0)
		return;

	Super::HandleDeath(DeathTag, NewCount);

	AStillHearPlayerController* PC = CastChecked<AStillHearPlayerController>(GetController());

	if (PC && PC->GetCameraEffectsComponent())
	{
		PC->GetCameraEffectsComponent()->StopAllEffects();
	}

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement(); // Disable movement when the character is dead

	// Disable input when the character is dead
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
		PlayerController->DisableInput(PlayerController);

	OnDeath.Broadcast();
}


void AStillHearMainCharacter::Revive()
{
	// Re-initialize camera state before re-enabling collision to prevent race condition overlaps from lerping
	CameraVolumesList.Empty();
	LastActiveCameraVolume = nullptr;
	bForceSnapOnNextCamera = true;

	// Undo ragdoll
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// Reset mesh transform to its original pose relative to the capsule
	GetMesh()->SetRelativeLocationAndRotation(DefaultMeshRelativeLocation, DefaultMeshRelativeRotation);

	// Re-enable capsule collision and movement
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->ClearAccumulatedForces();
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	UnCrouch(true);
	ResetJumpState();
	StopJumping();
	JumpCurrentCount = 0;

	if (CompanionComponent)
	{
		CompanionComponent->ResetCompanionLocation();
	}

	// Remove the death tag and cancel lingering abilities
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer DeathTagContainer;
		DeathTagContainer.AddTag(TAG_Status_Death);
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(DeathTagContainer);

		AbilitySystemComponent->CancelAbilities();
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Status_MainCharacter_Moving);
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_Status_MainCharacter_ForceMoving);
	}

	// Reset AnimInstance state
	if (MainAnimInstance)
	{
		MainAnimInstance->bIsFalling = false;
		MainAnimInstance->bIsCrouching = false;
		MainAnimInstance->bIsSprinting = false;
		MainAnimInstance->bIsClimbing = false;
		MainAnimInstance->StopAllMontages(0.0f);
	}

	// Make the mesh visible again (in case it was hidden during death)
	GetMesh()->SetHiddenInGame(false);

	if (AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetController()))

	{
		PC->ResetMovementState();
	}

	// Re-initialize camera and broadcast ready (same flow as initial spawn)
	GetCapsuleComponent()->UpdateOverlaps();
	CheckFirstCameraAtSpawn();
	FTimerHandle InitTimerHandle;
	GetWorldTimerManager().SetTimer(InitTimerHandle, [this]()
	{
		OnInitializedFinished.Broadcast();
	}, 0.5f, false);

	if (bStartSittingOnSpawn && MainAnimInstance)
	{
		bIsSitting = true;
		MainAnimInstance->bIsSitting = true;
	}
}

void AStillHearMainCharacter::StandUp()
{
	if (!MainAnimInstance)
		return;

	bIsSitting = false;
	MainAnimInstance->bIsSitting = false;
}

void AStillHearMainCharacter::SitDown()
{
	if (!SitDownMontage || bIsSitting || bIsCrouched || !MainAnimInstance)
		return;

	bIsSitting = true;
	
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		MoveComp->StopMovementImmediately();

	if (PCRef)
		PCRef->DisableInput(PCRef);

	const float MontageLength = MainAnimInstance->Montage_Play(SitDownMontage, SitDownMontage->RateScale);
	if (MontageLength <= 0.0f)
	{
		// Montage failed to play, switch directly to the seated-idle pose and unblock input
		MainAnimInstance->bIsSitting = true;

		if (PCRef)
			PCRef->EnableInput(PCRef);
		return;
	}

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ThisClass::OnSitDownBlendingOut);
	MainAnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, SitDownMontage);
}

void AStillHearMainCharacter::OnSitDownBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// If interrupted, let that sequence manage input/movement re-enabling
	if (bInterrupted)
		return;

	// Re-enable input so the Move action can still be processed
	if (PCRef)
		PCRef->EnableInput(PCRef);

	// Sit-down transition is blending out, switch to the looping seated-idle pose driven by the AnimInstance
	if (MainAnimInstance)
		MainAnimInstance->bIsSitting = true;
}

bool AStillHearMainCharacter::IsPerformingAbility() const
{
	if (!AbilitySystemComponent)
		return false;

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.IsActive())
			return true;
	}

	return false;
}

void AStillHearMainCharacter::Sprint() const
{
	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(SprintAbilityClass);
}

void AStillHearMainCharacter::StopSprinting()
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayTag EventTag = TAG_Event_InputReleased_Sprint;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearMainCharacter::CrouchAbility()
{
	if (!AbilitySystemComponent)
		return;

	if (!GetCharacterMovement()->IsCrouching())
		AbilitySystemComponent->TryActivateAbilityByClass(CrouchAbilityClass);
	else
	{
		const FGameplayTag EventTag = TAG_Event_InputReleased_Crouch;
		SendGameplayEventToSelf(EventTag);
	}
}

void AStillHearMainCharacter::StartCrouch() const
{
	if (!AbilitySystemComponent)
		return;

	if (!GetCharacterMovement()->IsCrouching())
		AbilitySystemComponent->TryActivateAbilityByClass(CrouchAbilityClass);
}

void AStillHearMainCharacter::ReleaseCrouch()
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayTag EventTag = TAG_Event_InputReleased_Crouch;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearMainCharacter::JumpAbility() const
{
	if (!AbilitySystemComponent || !PersonalizedCanJump())
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(JumpAbilityClass);
}

/*
void AStillHearMainCharacter::LowVault(const FVector& WallHeight,
                                       const FVector& LandingPoint)
{
	if (!AbilitySystemComponent || !CanJump())
		return;

	MotionWarpTarget = LandingPoint;
	WallHeightReferenceForClimbing = WallHeight;
	AbilitySystemComponent->TryActivateAbilityByClass(LowVaultAbilityClass);
}
*/

/*
void AStillHearMainCharacter::LowGetOnTop() const
{
        if (!AbilitySystemComponent || !CanJump())
                return;

        AbilitySystemComponent->TryActivateAbilityByClass(LowGetOnTopAbilityClass);
}
*/

void AStillHearMainCharacter::Climb(const FVector& ImpactPoint)
{
	if (!AbilitySystemComponent)
		return;

	MotionWarpTarget = ImpactPoint + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	AbilitySystemComponent->TryActivateAbilityByClass(ClimbAbilityClass);
}

// Parry ability activation
void AStillHearMainCharacter::Parry() const
{
	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(ParryAbilityClass);
}

// Resonance ability activation
void AStillHearMainCharacter::ActivateResonance() const
{
	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(ResonanceAbilityClass);
}

void AStillHearMainCharacter::DeactivateResonance()
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayTag EventTag = TAG_Event_MainCharacter_StopResonance;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearMainCharacter::ActivateResonanceInteraction()
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayTag EventTag = TAG_Event_MainCharacter_ActivateResonance;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearMainCharacter::TapInteraction()
{
	if (!AbilitySystemComponent)
		return;

	const bool bIsInteracting = AbilitySystemComponent->HasMatchingGameplayTag(TAG_GameplayAbility_MainCharacter_TapInteraction_Active);
	if (bIsInteracting)
	{
		const FGameplayTag EventTag = TAG_Event_StopInteraction;
		SendGameplayEventToSelf(EventTag);
	}
	else
		AbilitySystemComponent->TryActivateAbilityByClass(
			InteractionAbilityClass);
}

void AStillHearMainCharacter::StartSoundWave() const
{
	if (!AbilitySystemComponent)
		return;

	AbilitySystemComponent->TryActivateAbilityByClass(SoundWaveAbilityClass);
}

void AStillHearMainCharacter::ShootSoundWave()
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayTag EventTag = TAG_Event_Companion_ShootSoundWave;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearMainCharacter::InterruptSoundWave() const
{
	if (!AbilitySystemComponent)
		return;

	FGameplayTagContainer AbilityTagsToCancel;
	AbilityTagsToCancel.AddTag(TAG_GameplayAbility_Companion_SoundWave);

	AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
}

bool AStillHearMainCharacter::PersonalizedCanJump() const
{
	return CanJump() || bPersonalizedCanJump;
}

bool AStillHearMainCharacter::IsJumping() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_GameplayAbility_MainCharacter_Jump_Active);
}

TSubclassOf<UGameplayAbility> AStillHearMainCharacter::GetAbilityClassByType(EMainCharacterAbilityType AbilityType) const
{
	switch (AbilityType)
	{
	case EMainCharacterAbilityType::Jump:
		return JumpAbilityClass;
	case EMainCharacterAbilityType::Parry:
		return ParryAbilityClass;
	case EMainCharacterAbilityType::Resonance:
		return ResonanceAbilityClass;
	case EMainCharacterAbilityType::Sprint:
		return SprintAbilityClass;
	case EMainCharacterAbilityType::Crouch:
		return CrouchAbilityClass;
	case EMainCharacterAbilityType::LowVault:
		return LowVaultAbilityClass;
	case EMainCharacterAbilityType::Climb:
		return ClimbAbilityClass;
	case EMainCharacterAbilityType::SoundWave:
		return SoundWaveAbilityClass;
	case EMainCharacterAbilityType::Interaction:
		return InteractionAbilityClass;
	default:
		return nullptr;
	}
}

UMainCharacterAttributeSet* AStillHearMainCharacter::GetAttributeSet()
{
	return AttributeSet;
}

void AStillHearMainCharacter::SetDefaultLocomotionBlendSpace() const
{
	SetLocomotionBlendSpace(DefaultBlendSpace);
}

void AStillHearMainCharacter::SetLocomotionBlendSpace(UBlendSpace* BlendSpaceToApply) const
{
	if (!MainAnimInstance)
		return;

	MainAnimInstance->LocomotionBlendSpace = BlendSpaceToApply;
}

void AStillHearMainCharacter::SetAnimationClimbing(const bool Climbing) const
{
	if (!MainAnimInstance)
		return;

	MainAnimInstance->bIsClimbing = Climbing;
}

FVector AStillHearMainCharacter::GetWallNormalReference() const
{
	return WallNormalReferenceForClimbing;
}

FVector AStillHearMainCharacter::GetWallHeightReference() const
{
	return WallHeightReferenceForClimbing;
}

FVector AStillHearMainCharacter::GetMotionWarpTarget() const
{
	return MotionWarpTarget;
}

void AStillHearMainCharacter::SetMotionWarpTarget(const FVector& MotionTargetReference)
{
	MotionWarpTarget = MotionTargetReference;
}

FVector AStillHearMainCharacter::GetTargetPointLocation_Implementation()
{
	return GetActorLocation();
}

void AStillHearMainCharacter::AddCameraVolumeToList_Implementation(ACameraVolume* CameraVolume)
{
	// This check should always return false
	if (CameraVolumesList.Contains(CameraVolume))
		return;

	CameraVolumesList.Add(CameraVolume);

	// Since this function is called whenever the character enters a new volume, I
	// signal To my player controller that the input needs to be recalibrated,
	// independently of the fact That it finished calibrating the previous volume
	// or not.
	// Note: during a Level Sequence camera cut the controller may be temporarily
	// replaced or the cast may fail — guard against that to avoid a crash.
	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetController());
	if (PC)
	{
		PC->ResetInputUpdate();
	}

	CheckList();
}

UAISense_Sight::EVisibilityResult AStillHearMainCharacter::CanBeSeenFrom(const FCanBeSeenFromContext& Context, FVector& OutSeenLocation, int32& OutNumberOfLoSChecksPerformed, int32& OutNumberOfAsyncLosCheckRequested, float& OutSightStrength, int32* UserData, const FOnPendingVisibilityQueryProcessedDelegate* Delegate)
{
	// Define points to check for visibility to handle partial occlusion
	TArray<FVector> PointsToCheck;

	const FVector CharLoc = GetActorLocation();
	const float HalfHeight =
		GetCapsuleComponent()
			? GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 0.0f;

	// Center of the capsule
	PointsToCheck.Add(CharLoc);

	// Head
	PointsToCheck.Add(CharLoc + FVector(0.0f, 0.0f, HalfHeight * 0.85f));

	// Chest
	PointsToCheck.Add(CharLoc + FVector(0.0f, 0.0f, HalfHeight * 0.4f));

	// Hips
	PointsToCheck.Add(CharLoc - FVector(0.0f, 0.0f, HalfHeight * 0.4f));

	// Feet area
	PointsToCheck.Add(CharLoc - FVector(0.0f, 0.0f, HalfHeight * 0.85f));

	// The Context contains the ObserverLocation and other useful info
	const FVector ObserverLocation = Context.ObserverLocation;

	FCollisionQueryParams Params(FName(TEXT("AISightCheck")), true, Context.IgnoreActor);
	Params.AddIgnoredActor(this);

	for (const FVector& Point : PointsToCheck)
	{
		OutNumberOfLoSChecksPerformed++;

		// If the trace to this specific point is not blocked, the player is seen
		if (!GetWorld()->LineTraceTestByChannel(ObserverLocation, Point, ECC_Visibility, Params))
		{
			OutSeenLocation = Point;
			OutSightStrength = 1.0f;
			
			return UAISense_Sight::EVisibilityResult::Visible;
		}
	}

	return UAISense_Sight::EVisibilityResult::NotVisible;
}
