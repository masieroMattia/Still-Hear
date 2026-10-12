#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "StillHearPlayerController.generated.h"

struct FInputActionValue;
class UCommonActivatableWidget;
class UCameraEffectsComponent;
class AStillHearMainCharacter;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPauseStateChanged, bool, bIsPaused);

UCLASS()
class STILLHEAR_API AStillHearPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:

	UPROPERTY()
	TObjectPtr<AStillHearMainCharacter> CharacterRef;
	
protected:
	// Components
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UCameraEffectsComponent> CameraEffectsComponent;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCommonActivatableWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FGameplayTag MainMenuLayerTag;

	// Main Character input mapping
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping")
	TObjectPtr<class UInputMappingContext> KeyboardMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping")
	TObjectPtr<class UInputMappingContext> GamepadMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> ParryAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> ActivateResonanceAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> ResonanceInteractionAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|MainCharacter")
	TObjectPtr<UInputAction> InteractionAction;

	// Companion Character input mapping

	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|Companion")
	TObjectPtr<UInputAction> CompanionSoundWaveAction;
	//UPROPERTY(EditDefaultsOnly, Category = "InputMapping|Companion")
	//TObjectPtr<UInputAction> CompanionSoundWaveChargeAction;
	//UPROPERTY(EditDefaultsOnly, Category = "InputMapping|Companion")
	//TObjectPtr<UInputAction> CompanionSoundWaveShootAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|Companion")
	TObjectPtr<UInputAction> CompanionSoundWaveInterruptAction;
	UPROPERTY(EditDefaultsOnly, Category = "InputMapping|Companion")
	TObjectPtr<UInputAction> CompanionAimAction;

	// Climbing and Vaulting variables
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings")
	float MinimumWallHeightToStartVaulting;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings")
	float MaximumWallHeightToVault;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings")
	bool ShowLineTraces;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings")
	FName TagToCheckForClimbing;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float MinimumWallDepth;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float WallDistanceFromCharacter;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float WallDistanceFromCharacterLowVault;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float LineTraceVerticalHeight;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float VaultingWallDepth;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float MaximumStepHeightToEvaluateVault;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float VaultLandingOffset;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float EdgeGrabLeftHandOffset;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float EdgeGrabRightHandOffset;
	UPROPERTY(EditDefaultsOnly, Category = "ClimbingSettings|Advanced")
	float EdgeGrabFinalHeightOffset;
#pragma endregion 

#pragma region EVENTS
public:
	// Broadcast whenever SetPause changes the game's pause state
	UPROPERTY(BlueprintAssignable, Category = "Game")
	FOnPauseStateChanged OnPauseStateChanged;
#pragma endregion
	
#pragma region VARIABLES
private:
	// Recovering Controls Information
	UPROPERTY()
	TObjectPtr<class USaveSubsystem> SaveSubsystem;

	UPROPERTY()
	TObjectPtr<class UGameAudioSubsystem> AudioSubsystem;
	
	// For AI Affiliation
	FGenericTeamId TeamID;
	bool bIsInMenuMode = true;
	
	// Reference for Input Direction
	FVector CurrentRightInputDirection;
	bool bUpdateInputDirection = false;

	// Input Direction Lerp
	float InputDirectionLerpTime = 0.0f;
	float InputAdjustingTime = 0.0f;
	FVector InitialDirection;
	FVector FinalDirection;
	
	// Useful variables for climbing
	FVector WallLocation;
	FVector WallNormal;
	FVector WallHeight;
	FVector WallHeightAfterVaultingDistance;
	FTimerHandle EdgeGrabTimer;

	bool bIsForcedMoved = false;

	// Idle -> Sit handling
	FTimerHandle IdleSitTimerHandle;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	AStillHearPlayerController();
#pragma endregion
	
#pragma region METHODS
public:
	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	virtual bool SetPause(bool bPause, FCanUnpause CanUnpauseDelegate = FCanUnpause()) override;

	void ApplyControlsSettings();

	void ChangeCamera(class ACameraVolume* CameraVolume, const ACameraVolume* LastCameraVolume);
	void ResetInputUpdate() { bUpdateInputDirection = false; }
	void ResetMovementState();
	
	// IGenericTeamAgentInterface implementation for AI perception
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	//virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;

	// I put this function as public because the character calls it by himself in the Landed function, in case he jumps and doesn't find any edge grab to do
	void StopEdgeGrabTimer();

	void SetForcedMoved(const bool NewValue) { bIsForcedMoved = NewValue; }

	void TransitionToPossession(APawn* TargetPawn);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ReturnToMainMenu();
	
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void SkipCinematic();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void SetCinematicPause(bool bPause);

	UFUNCTION(BlueprintCallable, Category = "Components")
	UCameraEffectsComponent* GetCameraEffectsComponent() const { return CameraEffectsComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	
	void OnCinematicFinished();
	void OnCinematicStarted();

	UFUNCTION()
	void PushMainMenuWidget();

	// Main Character Input Handling
	void HandleMoveTriggered(const FInputActionValue & Val);
	void HandleMoveCompleted(const FInputActionValue & Val);
	void HandleSprintStarted(const FInputActionValue & Val);
	void HandleSprintCompleted(const FInputActionValue & Val);
	void HandleCrouchStarted(const FInputActionValue & Val);
	void HandleCrouchReleased(const FInputActionValue & Val);
	void HandleJumpStarted(const FInputActionValue & Val);
	void HandleParryTriggered(const FInputActionValue & Val);
	void HandleActivateResonanceTriggered(const FInputActionValue & Val);
	void HandleDeactivateResonanceCompleted(const FInputActionValue & Val);
	void HandleResonanceInteractionTriggered(const FInputActionValue & Val);
	void HandleInteractionTriggered(const FInputActionValue & Val);
	
	void UpdateInputDirection(const float& DeltaTime);

	// Idle -> Sit handling: resets the idle timer, and (if bCanWakeUp) wakes the character up if it was sitting
	void RegisterPlayerActivity(bool bCanWakeUp = true);
	// Called when the player has been idle for IdleTimeBeforeSit seconds
	void OnIdleSitTimeout();

	// Climbing and Vaulting handling
	bool ScanForWallInFrontOfPlayer(float ScanningDistance);
	bool IsWallHighEnoughForVaulting();
	bool CheckWallHeightRange(float MinRange, float MaxRange);
	bool CheckIfShouldVault();
	FVector CheckVaultLandingPoint() const;
	void StartEdgeGrabTimer();
	void EdgeGrabTraceOnTimerEnded();
	

	// Companion Input Handling
	void HandleCompanionSoundWaveChargeTriggered(const FInputActionValue & Val);
	void HandleCompanionSoundWaveShootCompleted(const FInputActionValue & Val);
	void HandleCompanionSoundWaveInterruptTriggered(const FInputActionValue & Val);
	void HandleCompanionSwitchTargetTriggered(const FInputActionValue & Val);


	// OnPosses
	virtual void OnPossess(APawn* InPawn) override;

	// Debug
	void DrawDebugRayCast(const FVector& Start, const FVector& End, bool Hit, const FHitResult& HitResult) const;
#pragma endregion 
};
