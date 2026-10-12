#include "Character/StillHearPlayerController.h"

#include "Flow/SceneManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "InputMappingContext.h"
#include "Camera/CameraVolume.h"
#include "StillHearGameInstance.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "CommonActivatableWidget.h"
#include "EnhancedInputSubsystems.h"
#include "UI/Subsystem/UISubsystem.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Audio/GameAudioSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/StillHearMainCharacter.h"
#include "TraceAndCollision/CustomCollision.h"
#include "Nodes/Actor/FlowNode_PlayLevelSequence.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"
#include "Animation/AnimInstances/MainCharacterAnimInstance.h"

void AStillHearPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		SaveSubsystem = GI->GetSubsystem<USaveSubsystem>();
		AudioSubsystem = GI->GetSubsystem<UGameAudioSubsystem>();
	}
	
	bool bStartInMenu = true;
	if (const ASceneManager* SceneManager = Cast<ASceneManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASceneManager::StaticClass())))
		bStartInMenu = SceneManager->GetStartInMenuMode();

	// If the persistent map was reloaded to hand off into a New Game or a Load Game don't flash the main menu
	if (const UStillHearGameInstance* StillHearGI = GetGameInstance<UStillHearGameInstance>())
	{
		if (StillHearGI->PendingNewGameSlotIndex > 0 || StillHearGI->PendingLoadGameSlotIndex > 0)
			bStartInMenu = false;
	}

	if (bStartInMenu)
	{
		bIsInMenuMode = true;

		// Play the main menu OST and silence the underlying streamed level's own audio
		if (AudioSubsystem)
		{
			AudioSubsystem->PushAudioState(TAG_Audio_State_MainMenu);
		}

		// Setup UI mapping context and mouse cursor for menu interaction
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		// Push the main menu widget when layout is ready
		if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>())
			{
				if (UISubsystem->GetPrimaryLayout())
					PushMainMenuWidget();
				else
					UISubsystem->OnUILayoutReady.AddDynamic(this, &AStillHearPlayerController::PushMainMenuWidget);
			}
		}
	}
	else
	{
		bIsInMenuMode = false;
		ApplyControlsSettings();
	}

	RegisterPlayerActivity();

	UFlowNode_PlayLevelSequence::OnPlaybackCompleted.AddUObject(this, &ThisClass::OnCinematicFinished);
	UFlowNode_PlayLevelSequence::OnPlaybackStarted.AddUObject(this, &ThisClass::OnCinematicStarted);
}

void AStillHearPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateInputDirection(DeltaSeconds);
}

void AStillHearPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	InputSubsystem->RemoveMappingContext(KeyboardMappingContext);
	InputSubsystem->RemoveMappingContext(GamepadMappingContext);
	
	UFlowNode_PlayLevelSequence::OnPlaybackCompleted.RemoveAll(this);
	UFlowNode_PlayLevelSequence::OnPlaybackStarted.RemoveAll(this);

	Super::EndPlay(Reason);
}

void AStillHearPlayerController::ApplyControlsSettings()
{
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	
	if (const USettingsSaveGame* Settings = SaveSubsystem->GetSaveSettings())
	{
		if (IsValid(Settings->GamepadContext))
		{
			InputSubsystem->RemoveMappingContext(GamepadMappingContext);
			GamepadMappingContext = Settings->GamepadContext;
		}
		
		bForceFeedbackEnabled = Settings->bActivateVibration;
		ForceFeedbackScale = Settings->VibrationIntensity / 100.f;
	}
	
	if (InputSubsystem)
	{
		InputSubsystem->AddMappingContext(KeyboardMappingContext, 0);
		InputSubsystem->AddMappingContext(GamepadMappingContext, 0);
	}
}

AStillHearPlayerController::AStillHearPlayerController()
{
	TeamID = FGenericTeamId(10);

	CameraEffectsComponent = CreateDefaultSubobject<UCameraEffectsComponent>(TEXT("CameraEffectsComponent"));
	
	PrimaryActorTick.bCanEverTick = true;

	ShowLineTraces = false;
	TagToCheckForClimbing = FName("Climb");
	MinimumWallHeightToStartVaulting = 70.0f;
	WallDistanceFromCharacter = 70.0f;
	MinimumWallDepth = 10.0f;
	MaximumWallHeightToVault = 95.0f;
	LineTraceVerticalHeight = 250.0f;
	VaultingWallDepth = 50.0f;
	MaximumStepHeightToEvaluateVault = 30.0f;
	//MaximumWallHeightToClimb = 220.0f;
	WallDistanceFromCharacterLowVault = 70.0f;
	VaultLandingOffset = 100.0f;
	EdgeGrabLeftHandOffset = 30.0f;
	EdgeGrabRightHandOffset = 30.0f;
	EdgeGrabFinalHeightOffset = 16.0f;
	
	CurrentRightInputDirection = FVector(0.0f);
}

void AStillHearPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Action Bindings

	// Main Character Input Actions
	if (MoveAction)
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleMoveTriggered
		);

		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::HandleMoveCompleted
		);
	}
	
	if (MoveAction)
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::HandleSprintCompleted
		);
	}
	

	if (SprintAction)
	{
		EnhancedInput->BindAction(
			SprintAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::HandleSprintStarted
		);
	}


	if (CrouchAction)
	{
		EnhancedInput->BindAction(
			CrouchAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::HandleCrouchStarted
		);
	}

	if (CrouchAction)
	{
		EnhancedInput->BindAction(
			CrouchAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::HandleCrouchReleased
		);
	}
	
	if (JumpAction)
	{
		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::HandleJumpStarted
		);
	}
	
	// Parry Input Action
	if (ParryAction)
	{
		EnhancedInput->BindAction(
			ParryAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleParryTriggered
		);
	}
	
	// Activate Resonance Input Action
	if (ActivateResonanceAction)
	{
		EnhancedInput->BindAction(
			ActivateResonanceAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleActivateResonanceTriggered
		);

		EnhancedInput->BindAction(
			ActivateResonanceAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::HandleDeactivateResonanceCompleted
		);
	}
	
	// Use Resonance Input Action
	if (ResonanceInteractionAction)
	{
		EnhancedInput->BindAction(
			ResonanceInteractionAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::HandleResonanceInteractionTriggered
		);
	}
	
	if (InteractionAction)
	{
		EnhancedInput->BindAction(
			InteractionAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleInteractionTriggered
		);
	}
	
	if (CompanionSoundWaveAction)
	{
		EnhancedInput->BindAction(
			CompanionSoundWaveAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::HandleCompanionSoundWaveChargeTriggered
		);
	}
	
	if (CompanionSoundWaveAction) 
	{
		EnhancedInput->BindAction(
			CompanionSoundWaveAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::HandleCompanionSoundWaveShootCompleted
		);
	}
	
	if (CompanionSoundWaveInterruptAction)
	{
		EnhancedInput->BindAction(
			CompanionSoundWaveInterruptAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleCompanionSoundWaveInterruptTriggered
		);
	}
	
	if (CompanionAimAction)
	{
		EnhancedInput->BindAction(
			CompanionAimAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::HandleCompanionSwitchTargetTriggered
		);
	}
	
}


void AStillHearPlayerController::ChangeCamera(class ACameraVolume* CameraVolume, const ACameraVolume* LastCameraVolume)
{
	if (!CameraVolume)
		return;
	
	SetAudioListenerOverride(CameraVolume->GetCamera(), FVector::ZeroVector, FRotator::ZeroRotator);

	if (bIsInMenuMode)
	{
		SetViewTargetWithBlend(
			CameraVolume,
			0.0f,
			CameraVolume->GetBlendFunction(),
			CameraVolume->GetBlendExp(),
			true
		);
		return;
	}
	
	const bool bShouldSnap = CameraVolume->IsSnappingToTarget();
	if (bShouldSnap)
	{
		SetViewTargetWithBlend(
			CameraVolume,
			0.0f,
			CameraVolume->GetBlendFunction(),
			CameraVolume->GetBlendExp(),
			true
		);
		return;
	}

	if (!LastCameraVolume || !LastCameraVolume->GetUseBlendParametersOnExit())
	{		
		SetViewTargetWithBlend(
			CameraVolume,
			CameraVolume->GetBlendTimeOnEnter(),
			CameraVolume->GetBlendFunction(),
			CameraVolume->GetBlendExp(),
			true
		);
		
		return;
	}
	
	SetViewTargetWithBlend(CameraVolume, LastCameraVolume->GetBlendTimeOnExit(), LastCameraVolume->GetBlendFunction(), LastCameraVolume->GetBlendExp(), true);
}

void AStillHearPlayerController::OnCinematicStarted()
{
	if (!CharacterRef)
		return;

	if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
	{
		ASC->CancelAbilities();
	}
}

void AStillHearPlayerController::OnCinematicFinished()
{
	if (CharacterRef)
	{
		if (UCapsuleComponent* Capsule = CharacterRef->GetCapsuleComponent())
		{
			Capsule->UpdateOverlaps();
		}
		CharacterRef->SetForceSnapOnNextCamera(true);
		CharacterRef->ResetLastActiveCameraVolume();
		CharacterRef->CheckList();
	}
}

void AStillHearPlayerController::HandleMoveTriggered(const FInputActionValue & Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	RegisterPlayerActivity();

	if (bIsForcedMoved)
		return;

	const ACameraVolume* CameraVolume = CharacterRef->GetLastActiveCameraVolume();
	if (!CameraVolume)
		return;

	const FVector2D InputValue = Val.Get<FVector2D>();

	// First time you move, beginning of the game or respawn
	if (CurrentRightInputDirection == FVector::ZeroVector)
	{
		CurrentRightInputDirection = CameraVolume->GetRightDirection();
	}
	// Changing volumes at runtime
	else if (CurrentRightInputDirection != CameraVolume->GetRightDirection() && !bUpdateInputDirection)
	{
		InitialDirection = CurrentRightInputDirection;
		FinalDirection = CameraVolume->GetRightDirection();
		InputAdjustingTime = CameraVolume->GetHasPlayerAdjustedToInput() ? 0.01f : CameraVolume->GetInputAdjustingTime();
		bUpdateInputDirection = true;
	}
	
	const FVector ForwardInputDirection = CurrentRightInputDirection.GetSafeNormal().Cross(FVector::UpVector);
	
	GetCharacter()->AddMovementInput(CurrentRightInputDirection, InputValue.X);
	GetCharacter()->AddMovementInput(ForwardInputDirection, InputValue.Y);

	// Add moving status tag while moving
	if (CharacterRef)
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			if (!ASC->HasMatchingGameplayTag(TAG_Status_MainCharacter_Moving))
			{
				ASC->AddLooseGameplayTag(TAG_Status_MainCharacter_Moving);
			}
		}
	}
}

void AStillHearPlayerController::HandleMoveCompleted(const FInputActionValue& Val)
{
	// Remove moving status tag when input stops
	if (CharacterRef)
	{
		if (UAbilitySystemComponent* ASC = CharacterRef->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(TAG_Status_MainCharacter_Moving);
		}
	}
}

/*
void AStillHearPlayerController::ResetInputDirection(const FInputActionValue& Val)
{
	CurrentRightInputDirection = FVector(0.0f);
}
*/

void AStillHearPlayerController::HandleSprintStarted(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->Sprint();
}

void AStillHearPlayerController::HandleSprintCompleted(const FInputActionValue& Val)
{
	if (!GetPawn())
		return;

	CastChecked<AStillHearMainCharacter>(GetCharacter())->StopSprinting();
}

void AStillHearPlayerController::HandleCrouchStarted(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block crouch while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->StartCrouch();
}

void AStillHearPlayerController::HandleCrouchReleased(const FInputActionValue& Val)
{
	if (!GetPawn())
		return;

	CastChecked<AStillHearMainCharacter>(GetCharacter())->ReleaseCrouch();
}

bool AStillHearPlayerController::ScanForWallInFrontOfPlayer(const float ScanningDistance)
{
	// Hit Result declaration
	FHitResult HitResult;

	// Start Vector
	FVector StartLocation = GetCharacter()->GetActorLocation();

	// I put the start location of the line trace on the feet of the character, with a small offset so it's not exactly on the ground 
	StartLocation.Z -=  GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - MinimumWallHeightToStartVaulting;

	// End Vector
	const FVector EndLocation = StartLocation + GetCharacter()->GetActorForwardVector() * ScanningDistance;

	// Collision Parameters
	/*
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	*/
	
	// Ignore Parameters
	FCollisionQueryParams IgnoreParams;
	IgnoreParams.AddIgnoredActor(GetCharacter());
	
	// Line Trace
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECustomCollision::Climb,
		IgnoreParams
	);

	if (ShowLineTraces)
		DrawDebugRayCast(StartLocation, EndLocation, bHit, HitResult);
	
	if (bHit)
	{
		const AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) && HitActor->ActorHasTag(TagToCheckForClimbing))
		{
			WallLocation = HitResult.Location;
			WallNormal = HitResult.Normal;
			return true;	
		}
	}

	WallLocation = FVector::ZeroVector;
	WallNormal = FVector::ZeroVector;
	return false;
}

bool AStillHearPlayerController::IsWallHighEnoughForVaulting()
{
	return CheckWallHeightRange(MinimumWallHeightToStartVaulting, MaximumWallHeightToVault);
}

bool AStillHearPlayerController::CheckWallHeightRange(const float MinRange, const float MaxRange)
{
	const FVector Offset = WallNormal.GetSafeNormal() * -MinimumWallDepth;
	
	// Hit Result declaration
	FHitResult HitResult;

	// Start Vector
	FVector StartLocation = WallLocation + Offset;

	// I put the start location of the line trace on the feet of the character, with a small offset so it's not exactly on the ground 
	StartLocation.Z += LineTraceVerticalHeight;

	// End Vector
	const FVector EndLocation = WallLocation + Offset;

	// Collision Parameters
	/*
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	*/
	
	// Ignore Parameters
	FCollisionQueryParams IgnoreParams;
	IgnoreParams.AddIgnoredActor(GetCharacter());
	
	// Line Trace
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECustomCollision::Climb,
		IgnoreParams
	);

	if (ShowLineTraces)
		DrawDebugRayCast(StartLocation, EndLocation, bHit, HitResult);
	
	if (bHit)
	{
		WallHeight = HitResult.ImpactPoint;
		const FVector MotionWarpTarget = WallHeight + GetCharacter()->GetActorForwardVector() * GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleRadius();
		CastChecked<AStillHearMainCharacter>(GetCharacter())->SetMotionWarpTarget(MotionWarpTarget);
	}
	else
		return false;

	FVector CharacterFeet = GetCharacter()->GetActorLocation();
	CharacterFeet.Z -= GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	if ((WallHeight - CharacterFeet).Z >= MinRange &&
		(WallHeight - CharacterFeet).Z <= MaxRange)
		return true;

	return false;
}

bool AStillHearPlayerController::CheckIfShouldVault()
{
	const FVector Offset = WallNormal.GetSafeNormal() * -VaultingWallDepth;
	
	// Hit Result declaration
	FHitResult HitResult;

	// Start Vector
	FVector StartLocation = WallLocation + Offset;

	// I put the start location of the line trace on the feet of the character, with a small offset so it's not exactly on the ground 
	StartLocation.Z += LineTraceVerticalHeight;

	// End Vector
	const FVector EndLocation = WallLocation + Offset;

	// Collision Parameters
	/*
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	*/
	
	// Ignore Parameters
	FCollisionQueryParams IgnoreParams;
	IgnoreParams.AddIgnoredActor(GetCharacter());
	
	// Line Trace
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECustomCollision::Climb,
		IgnoreParams
	);

	if (ShowLineTraces)
		DrawDebugRayCast(StartLocation, EndLocation, bHit, HitResult);
	
	if (bHit)
	{
		WallHeightAfterVaultingDistance = HitResult.Location;
		if ((WallHeight - WallHeightAfterVaultingDistance).Z >= MaximumStepHeightToEvaluateVault)
			return true;
		
		return false;
	}

	return true;
}

FVector AStillHearPlayerController::CheckVaultLandingPoint() const
{
	
	// Hit Result declaration
	FHitResult HitResult;

	// Start Vector
	const FVector StartLocation = WallHeight + GetCharacter()->GetActorForwardVector() * VaultLandingOffset;
	FVector JustInCaseLocation = StartLocation;
	JustInCaseLocation.Z = 0;

	// End Vector
	FVector EndLocation = StartLocation;
	EndLocation.Z -= 1000.0f;

	// Collision Parameters
	/*
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	*/
	
	// Ignore Parameters
	FCollisionQueryParams IgnoreParams;
	IgnoreParams.AddIgnoredActor(GetCharacter());
	
	// Line Trace
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECustomCollision::Climb,
		IgnoreParams
	);
	
	if (ShowLineTraces)
		DrawDebugRayCast(StartLocation, EndLocation, bHit, HitResult);

	return bHit ? HitResult.Location : JustInCaseLocation;
}

void AStillHearPlayerController::StartEdgeGrabTimer()
{
	constexpr float Time = 0.05f;
	constexpr bool bLoop = true;

	GetWorldTimerManager().SetTimer(
		EdgeGrabTimer,
		this,
		&ThisClass::EdgeGrabTraceOnTimerEnded,
		Time,
		bLoop
	);
}

/*
void AStillHearPlayerController::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	IGenericTeamAgentInterface::SetGenericTeamId(InTeamID);

	if (InTeamID != TeamID)
		TeamID = InTeamID;
}
*/

void AStillHearPlayerController::StopEdgeGrabTimer()
{
	if (GetWorldTimerManager().IsTimerActive(EdgeGrabTimer))
		GetWorldTimerManager().ClearTimer(EdgeGrabTimer);
}

void AStillHearPlayerController::EdgeGrabTraceOnTimerEnded()
{
	if (!IsValid(CharacterRef))
	{
		StopEdgeGrabTimer();
		return;
	}

	// I only want to make the trace when the character is falling down, not when he is raising
	/*
	if (!(CharacterRef->GetCharacterMovement()->Velocity.Z < 0))
		return;
	*/

	FHitResult HitResult;

	// I start with the first box trace, which checks if a wall is in front of the character's face during the fall
	FVector StartLocation = CharacterRef->GetActorLocation() + CharacterRef->GetActorForwardVector() * 45.0f;
	FVector EndLocation = StartLocation;
	StartLocation.Z += 100.0f;
	EndLocation.Z += 25.0f;

	// Defining box size
	FVector BoxHalfSize = FVector(25.0f, 5.0f, 1.0f);

	// Orientation Parameter
	FRotator Orientation = CharacterRef->GetActorRotation();

	// Collision Parameters
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

	// Ignore Parameters
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(CharacterRef);

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
		BoxHalfSize,
		Orientation,
		UEngineTypes::ConvertToTraceType(ECustomCollision::Climb),
		false,
		ActorsToIgnore,
		ShowLineTraces ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);

	if (!bHit || HitResult.Distance <= 0 || !IsValid(HitResult.GetActor()) || !HitResult.GetActor()->ActorHasTag(FName(TagToCheckForClimbing)))
		return;

	// If the first box trace hits and the distance was more than 0, I proceed with the second box trace
	// The second box trace localizes the edge of the wall which I should end up grabbing

	// I only calculate Start, End, Orientation and BoxSize. Everything else will be the same to the first cast.
	StartLocation = CharacterRef->GetActorLocation() + CharacterRef->GetActorForwardVector() * 5.0f;
	StartLocation.Z = HitResult.ImpactPoint.Z;
	EndLocation = HitResult.Location + CharacterRef->GetActorForwardVector() * 5.0f;
	Orientation = FRotator(0, 0, 0);
	BoxHalfSize = FVector(5.0f, 5.0f, 5.0f);

	bHit = UKismetSystemLibrary::BoxTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
		BoxHalfSize,
		Orientation,
		UEngineTypes::ConvertToTraceType(ECustomCollision::Climb),
		false,
		ActorsToIgnore,
		ShowLineTraces ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);

	if (!bHit)
		return;

	/*
	 * If the second box cast also has success, I can proceed with the edge grab.
	 * Before doing that, I set the AnimInstance variables that the Character will use to apply
	 * Inverse Kinematic to his hands
	 *
	 * To find the points, I need to take the edge of the wall that I calculated with the box trace, and apply some offset to the
	 * left and to the right
	 */

	WallNormal = HitResult.Normal.GetSafeNormal();
	const FRotator HandRotation = (-WallNormal).Rotation();

	const FVector WallRight = WallNormal.Cross(FVector::UpVector);
	const FVector LeftHandPosition = HitResult.ImpactPoint - WallRight * EdgeGrabLeftHandOffset;
	const FVector RightHandPosition = HitResult.ImpactPoint + WallRight * EdgeGrabRightHandOffset;
	
	USkeletalMeshComponent* Mesh = CharacterRef->GetMesh();
	if (Mesh && Mesh->GetAnimInstance())
	{
		IIKTargetReceiver* IKReceiver = Cast<IIKTargetReceiver>(Mesh->GetAnimInstance());
		if (IKReceiver)
		{
			const FTransform LeftTransform(HandRotation, LeftHandPosition);
			const FTransform RightTransform(HandRotation, RightHandPosition);

			IKReceiver->UpdateIKTargets(LeftTransform, RightTransform);
		}
	}

	CharacterRef->Climb(HitResult.ImpactPoint + FVector(0.f, 0.f, EdgeGrabFinalHeightOffset));
	StopEdgeGrabTimer();
}

void AStillHearPlayerController::HandleJumpStarted(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	// The low vaulting ability has a shorter check distance compared to other climbing abilities
	/*
	if (ScanForWallInFrontOfPlayer(WallDistanceFromCharacterLowVault))
	{
		if (IsWallHighEnoughForVaulting() && CheckIfShouldVault())
		{
			const FVector LandingPoint = CheckVaultLandingPoint();
			CastChecked<AStillHearMainCharacter>(GetCharacter())->LowVault(WallHeight, LandingPoint);
			return;
		}
	}
	*/

	// Temporarily decided to not use the Low Get On Top ability since it doesn't really work as expected

	/*
	if (ScanForWallInFrontOfPlayer(WallDistanceFromCharacter))
	{
		if (IsWallHighEnoughForVaulting() && !CheckIfShouldVault())
		{
			CastChecked<AStillHearMainCharacter>(GetCharacter())->LowGetOnTop();
			return;
		}
	}
	*/
	
	// Jump
	CharacterRef->JumpAbility();
	if (CharacterRef->IsJumping())
		StartEdgeGrabTimer();
}


void AStillHearPlayerController::HandleParryTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->Parry();
}

void AStillHearPlayerController::HandleActivateResonanceTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->ActivateResonance();
}

void AStillHearPlayerController::HandleDeactivateResonanceCompleted(const FInputActionValue& Val)
{
	if (!GetPawn())
		return;
	
	
	CastChecked<AStillHearMainCharacter>(GetCharacter())->DeactivateResonance();
}

void AStillHearPlayerController::HandleResonanceInteractionTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->ActivateResonanceInteraction();
}

void AStillHearPlayerController::HandleInteractionTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);

	CastChecked<AStillHearMainCharacter>(GetCharacter())->TapInteraction();
}

void AStillHearPlayerController::ResetMovementState()
{
	CurrentRightInputDirection = FVector::ZeroVector;
	bUpdateInputDirection = false;
	InputDirectionLerpTime = 0.0f;
}

void AStillHearPlayerController::RegisterPlayerActivity(bool bCanWakeUp)
{
	if (bCanWakeUp && CharacterRef && CharacterRef->IsSitting())
	{
		CharacterRef->StandUp();
	}

	if (!CharacterRef || !CharacterRef->IsIdleSitEnabled())
		return;

	// Avoid re-arming the timer every tick while an input is held
	// only refresh it once a meaningful amount of time has passed since the last reset
	FTimerManager& TimerManager = GetWorldTimerManager();
	if (TimerManager.IsTimerActive(IdleSitTimerHandle) && TimerManager.GetTimerElapsed(IdleSitTimerHandle) < 1.0f)
		return;

	TimerManager.SetTimer(IdleSitTimerHandle, this, &ThisClass::OnIdleSitTimeout, CharacterRef->GetIdleTimeBeforeSit(), false);
}

void AStillHearPlayerController::OnIdleSitTimeout()
{
	if (!CharacterRef || !CharacterRef->IsIdleSitEnabled())
		return;

	// Don't sit down during menus, cinematics, or while any ability is active - check again later instead
	if (bIsInMenuMode || IsMoveInputIgnored() || CharacterRef->IsPerformingAbility())
	{
		GetWorldTimerManager().SetTimer(IdleSitTimerHandle, this, &ThisClass::OnIdleSitTimeout, CharacterRef->GetIdleTimeBeforeSit(), false);
		return;
	}

	CharacterRef->SitDown();

	if (!CharacterRef->IsSitting())
		GetWorldTimerManager().SetTimer(IdleSitTimerHandle, this, &ThisClass::OnIdleSitTimeout, CharacterRef->GetIdleTimeBeforeSit(), false);
}

void AStillHearPlayerController::UpdateInputDirection(const float& DeltaTime)
{
	if (!CharacterRef)
		return;
	
	ACameraVolume* CameraVolume = CharacterRef->GetLastActiveCameraVolume();

	if (!CameraVolume)
		return;

	if (!bUpdateInputDirection)
		return;

	InputDirectionLerpTime += DeltaTime;
	const float Alpha = InputDirectionLerpTime / InputAdjustingTime;
	CurrentRightInputDirection = FMath::Lerp(InitialDirection, FinalDirection, Alpha);

	if (InputDirectionLerpTime >= InputAdjustingTime)
	{
		InputDirectionLerpTime = 0.0f;
		CurrentRightInputDirection = FinalDirection;
		bUpdateInputDirection = false;

		// If the input has to follow the camera rotation, after it's adjusted I force the time to adjust to a very small value
		if (CameraVolume->GetInputFollowsCamera())
			CameraVolume->SetHasPlayerAdjustedToInput(true);
	}
}


void AStillHearPlayerController::HandleCompanionSoundWaveChargeTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);
	
	CharacterRef->StartSoundWave();
}

void AStillHearPlayerController::HandleCompanionSoundWaveShootCompleted(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;
	
	RegisterPlayerActivity(false);

	CharacterRef->ShootSoundWave();
}

void AStillHearPlayerController::HandleCompanionSoundWaveInterruptTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn())
		return;

	// Block abilities while seated
	if (CharacterRef && CharacterRef->IsSitting())
		return;
	
	RegisterPlayerActivity(false);

	CharacterRef->InterruptSoundWave();
}

void AStillHearPlayerController::HandleCompanionSwitchTargetTriggered(const FInputActionValue& Val)
{
	if (!InputEnabled() || IsMoveInputIgnored())
		return;

	if (!GetPawn() || !CharacterRef || !CharacterRef->GetAbilitySystemComponent())
		return;

	// Block abilities while seated
	if (CharacterRef->IsSitting())
		return;

	RegisterPlayerActivity(false);
	
	const bool bIsAiming = CharacterRef->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_Status_Companion_Aiming);
	if (!bIsAiming)
		return;
	
	const FVector2D AimInput = Val.Get<FVector2D>();
	if (AimInput.IsNearlyZero(0.1f))
		return;
	
	const FVector2D InputDir2D = FVector2D(AimInput.X, -AimInput.Y).GetSafeNormal();
	
	if (!GetCharacter())
		return;
	
	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_Companion_SwitchSoundWaveTarget;
	
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	LocationData->TargetLocation.LiteralTransform.SetLocation(FVector(InputDir2D.X, InputDir2D.Y, 0.f));
	EventData.TargetData.Add(LocationData);
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPawn(), EventData.EventTag, EventData);
	
}

void AStillHearPlayerController::SkipCinematic()
{
	UFlowNode_PlayLevelSequence::OnSkipRequested.Broadcast();
}

void AStillHearPlayerController::SetCinematicPause(const bool bPause)
{
	TArray<AActor*> SequenceActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALevelSequenceActor::StaticClass(), SequenceActors);

	for (AActor* Actor : SequenceActors)
	{
		const ALevelSequenceActor* SeqActor = Cast<ALevelSequenceActor>(Actor);
		if (!SeqActor) 
			continue;

		ULevelSequencePlayer* SeqPlayer = SeqActor->GetSequencePlayer();
		if (!SeqPlayer) 
			continue;

		if (bPause)
			SeqPlayer->Pause();
		else if (SeqPlayer->IsPaused())
			SeqPlayer->Play();
	}
}

void AStillHearPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CharacterRef = CastChecked<AStillHearMainCharacter>(GetCharacter());
}


void AStillHearPlayerController::DrawDebugRayCast(const FVector& Start, const FVector& End, const bool Hit, const FHitResult& HitResult) const
{
	
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		Hit ? FColor::Green : FColor::Red,
		false,      
		5.0f,       
		0,
		2.0f        
	);

	if (Hit)
	{
		// Point of the impact
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, 5.0f);

		// Normal of the impact
		DrawDebugDirectionalArrow(
			GetWorld(),
			HitResult.ImpactPoint,
			HitResult.ImpactPoint + HitResult.ImpactNormal * 50.0f,
			20.0f,
			FColor::Cyan,
			false,
			1.0f,
			0,
			2.0f
		);
	}
}

void AStillHearPlayerController::TransitionToPossession(APawn* TargetPawn)
{
	if (!TargetPawn)
		return;

	bIsInMenuMode = false;

	// Fade out the main menu OST
	if (AudioSubsystem)
	{
		AudioSubsystem->PopAudioState(TAG_Audio_State_MainMenu);
	}

	// Possess the character (since view target was already the ACameraVolume, there is no camera jump)
	Possess(TargetPawn);

	// Hide mouse cursor
	SetShowMouseCursor(false);

	// Re-enable active gameplay input mode
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	// Re-enable character physics/movement input binding
	if (AStillHearMainCharacter* PossessedCharacter = Cast<AStillHearMainCharacter>(TargetPawn))
	{
		CharacterRef = PossessedCharacter;
		CharacterRef->EnableInput(this);
	}

	// Activate gameplay input mapping contexts
	ApplyControlsSettings();
}

void AStillHearPlayerController::PushMainMenuWidget()
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
		return;

	UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
		return;

	if (!MainMenuWidgetClass || !MainMenuLayerTag.IsValid())
		return;

	// Push the widget to the designated layer
	UISubsystem->PushWidgetToLayer(MainMenuLayerTag, MainMenuWidgetClass, false);

	bShowMouseCursor = true;
}

bool AStillHearPlayerController::SetPause(const bool bPause, const FCanUnpause CanUnpauseDelegate)
{
	if (bPause)
	{
		if (CharacterRef)
		{
			CharacterRef->DeactivateResonance();
			CharacterRef->InterruptSoundWave();
		}
	}

	if (AudioSubsystem)
	{
		const float Multiplier = bPause ? AudioSubsystem->GetPauseDuckMultiplier() : 1.0f;
		AudioSubsystem->SetMusicDucking(Multiplier, AudioSubsystem->GetDuckFadeTime());
	}

	const bool bResult = Super::SetPause(bPause, CanUnpauseDelegate);

	OnPauseStateChanged.Broadcast(bPause);

	return bResult;
}

void AStillHearPlayerController::ReturnToMainMenu()
{
	SetPause(false);

	bIsInMenuMode = true;
	CharacterRef = nullptr;

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		InputSubsystem->RemoveMappingContext(KeyboardMappingContext);
		InputSubsystem->RemoveMappingContext(GamepadMappingContext);
	}

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (ASceneManager* SceneManager = Cast<ASceneManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASceneManager::StaticClass())))
	{
		SceneManager->ReturnToMainMenu();
	}

	// Back to the main menu OST and silence the level's own audio
	if (AudioSubsystem)
	{
		AudioSubsystem->PushAudioState(TAG_Audio_State_MainMenu);
	}

	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UUISubsystem>())
		{
			UISubsystem->ClearLayer(TAG_UI_Layer_Game);
			UISubsystem->ClearLayer(TAG_UI_Layer_Menu);
			UISubsystem->ClearLayer(TAG_UI_Layer_Window);
			UISubsystem->ClearLayer(TAG_UI_Layer_Modal);
		}
	}

	PushMainMenuWidget();
}