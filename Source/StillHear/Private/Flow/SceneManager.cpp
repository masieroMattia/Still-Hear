#include "Flow/SceneManager.h"

#include "FlowAsset.h"
#include "FlowSubsystem.h"
#include "Camera/CameraVolume.h"
#include "StillHearGameInstance.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "SaveSystem/SaveSubsystem.h"
#include "UI/Subsystem/UISubsystem.h"
#include "SaveSystem/SaveGameObject.h"
#include "UI/Slate/SLoadingScreenWidget.h"
#include "Character/StillHearMainCharacter.h"
#include "Subsystems/TimeManagementSubsystem.h"
#include "Character/StillHearPlayerController.h"
#include "ProjectSettings/LoadingScreenSettings.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

// Sets default values
ASceneManager::ASceneManager()
{
	PrimaryActorTick.bCanEverTick = false;
	SaveIdComponent = CreateDefaultSubobject<USaveIdComponent>(TEXT("SaveIdComponent"));
	bSceneInitialized = false;
}

bool ASceneManager::CanActivateCheckpoint(int32 Priority) const
{
	return Priority > CurrentCheckpointPriority;
}

void ASceneManager::UpdateRuntimeVisibility(const TArray<FName>& LevelsToShow, const TArray<FName>& LevelsToHide)
{
	// Update the runtime tracking array
	for (const FName& Level : LevelsToHide)
	{
		RuntimeVisibleLevels.Remove(Level);
	}
	for (const FName& Level : LevelsToShow)
	{
		RuntimeVisibleLevels.AddUnique(Level);
	}

	// Apply the actual visibility changes
	HideStreamingLevels(LevelsToHide);
	ShowStreamingLevels(LevelsToShow);
}

void ASceneManager::UpdateSavedState(const TArray<FName>& LevelsToShow, const TArray<FName>& LevelsToHide, const FVector& Location, const FRotator& Rotation, int32 Priority)
{
	// 1. Apply runtime visibility (visual transition)
	UpdateRuntimeVisibility(LevelsToShow, LevelsToHide);

	// 2. Update the persistent saved state
	CurrentCheckpointPriority = Priority;
	PlayerStartLocation = Location;
	PlayerStartRotation = Rotation;

	// Rebuild the saved levels list from the current RuntimeVisibleLevels
	StreamingLevelsLoadedFromSave.Empty();
	StreamingLevelsLoadedFromSave.Append(RuntimeVisibleLevels);

	// 3. Snapshot all interactables so death-reset restores to this state
	if (const UWorld* World = GetWorld())
	{
		if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnCheckpointSnapshot.Broadcast();
		}
	} 

	// 4. Persist to disk
	TrySaveGame();
}

// Called when the game starts or when spawned
void ASceneManager::BeginPlay()
{
	Super::BeginPlay();

	// For Debug
	if (bBypassSaveGame)
		bSaveLoaded = true;

	if (bStartInMenuMode)
	{
		bIsInMenuMode = true;
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
			{
				// (New Game placeholder save / Load Game): skip loading here, let OnAllLevelsShown's hand-off do it once the world has settled
				const UStillHearGameInstance* StillHearGI = Cast<UStillHearGameInstance>(GI);
				const bool bPendingReload = StillHearGI && (StillHearGI->PendingNewGameSlotIndex > 0 || StillHearGI->PendingLoadGameSlotIndex > 0);

				int32 LatestSlot = SaveSub->GetLatestSaveSlotIndex();
				if (!bPendingReload && LatestSlot > 0)
					SaveSub->LoadFromSlot(LatestSlot, 0, false);
				else
					bSaveLoaded = true;
			}
		}
	}
	else
	{
		bIsInMenuMode = false;
		if (!bSaveLoaded)
		{
			if (const UGameInstance* GI = GetGameInstance())
			{
				if (const USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
				{
					if (SaveSub->GetCurrentSlotSave() <= 0)
					{
						bSaveLoaded = true;
					}
					else
					{
						FTimerHandle FallbackHandle;
						GetWorldTimerManager().SetTimer(FallbackHandle, [this]()
						{
							if (!bSaveLoaded)
							{
								bSaveLoaded = true;
								TryInitializeScene();
							}
						}, 0.1f, false);
					}
				}
			}
			else
			{
				bSaveLoaded = true;
			}
		}
	}

	PendingLevelsToLoad = LevelsStreamingToLoad.Num();

	if (PendingLevelsToLoad == 0)
	{
		OnAllLevelsReady();
		return;
	}

	for (int32 i = 0; i < LevelsStreamingToLoad.Num(); ++i)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		LatentInfo.ExecutionFunction = FName("OnLevelLoaded");
		LatentInfo.Linkage = 0;
		LatentInfo.UUID = i + 1;

		UGameplayStatics::LoadStreamLevel(
			this,
			LevelsStreamingToLoad[i],
			false,
			false,
			LatentInfo);
	}
}

void ASceneManager::OnLevelLoaded()
{
	PendingLevelsToLoad--;
	if (PendingLevelsToLoad <= 0)
	{
		OnAllLevelsReady();
	}
}

void ASceneManager::OnLevelShown()
{
	IncrementShownCount();
}

void ASceneManager::IncrementShownCount()
{
	// Prevent processing if already reached the expected count
	if (CurrentLevelsShown >= TotalLevelsToShow)
		return;

	++CurrentLevelsShown;

	if (CurrentLevelsShown < TotalLevelsToShow)
		return;

	// All levels in the batch are now visible – dispatch based on context
	OnAllLevelsShown();
}

void ASceneManager::OnAllLevelsShown()
{
	const EShowLevelsContext CurrentContext = ActiveShowContext;

	// Reload was triggered to hand off into New Game / Load Game — skip the menu and
	// continue the load-and-transition pipeline now that the pristine world has settled.
	if (CurrentContext == EShowLevelsContext::InitialLoad && bStartInMenuMode && bIsInMenuMode)
	{
		int32 PendingNewGameSlot = -1;
		int32 PendingLoadGameSlot = -1;
		if (UStillHearGameInstance* GI = GetWorld()->GetGameInstance<UStillHearGameInstance>())
		{
			PendingNewGameSlot = GI->PendingNewGameSlotIndex;
			PendingLoadGameSlot = GI->PendingLoadGameSlotIndex;
			GI->PendingNewGameSlotIndex = -1;
			GI->PendingLoadGameSlotIndex = -1;
		}

		if (PendingNewGameSlot > 0)
		{
			LoadSlotAndTransition(PendingNewGameSlot);
			return;
		}

		if (PendingLoadGameSlot > 0)
		{
			LoadSlotAndTransition(PendingLoadGameSlot, /*bAllowFullReload=*/false);
			return;
		}
	}

	switch (CurrentContext)
	{
		case EShowLevelsContext::InitialLoad:
			if (bStartInMenuMode && bIsInMenuMode)
			{
				bool bHasSave = false;
				if (const UGameInstance* GI = GetGameInstance())
					if (const USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
						bHasSave = SaveSub->GetCurrentSlotSave() > 0;
				SetupMenuView(bHasSave);
			}
			else if (!PlayerCharacter)
			{
				SpawnCharacter();
			}
			break;

		case EShowLevelsContext::Respawn:
			ResetCharacter();

			// Re-enable input
			if (AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(PlayerCharacter))
			{
				APlayerController* PC = Cast<APlayerController>(MainCharacter->GetController());
				if (PC)
					PC->EnableInput(PC);
				MainCharacter->OnInitializedFinished.AddUniqueDynamic(this, &ASceneManager::OnCharacterInitialized);
			}
			break;

		case EShowLevelsContext::LoadGame:
			if (CurrentCheckpointPriority == -1)
			{
				if (const UWorld* World = GetWorld())
				{
					if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
					{
						GI->bIsNewGameResetting = true;
						GI->OnClearCheckpointState.Broadcast();
					}
				}
				ResetActors();
				if (const UWorld* World = GetWorld())
				{
					if (UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
					{
						GI->bIsNewGameResetting = false;
					}
				}
			}
			TransitionToGameplay();

			if (UStillHearGameInstance* GI = GetWorld()->GetGameInstance<UStillHearGameInstance>())
			{
				GI->bIsSpawningNewGame = false;
			}
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				if (PC->PlayerCameraManager)
					PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, false);
			}
			break;

		case EShowLevelsContext::RuntimeTransition:
			// Normal checkpoint transition – nothing extra to do
			break;
	}
	
	if (CurrentContext == EShowLevelsContext::InitialLoad)
	{
		ResetActors();

		// Start or restore the root flow graph if configured, ONLY if we are NOT starting in menu mode
		if (!bStartInMenuMode)
			StartFlowRoot();
	}

	if (CurrentContext == EShowLevelsContext::InitialLoad || CurrentContext == EShowLevelsContext::Respawn || CurrentContext == EShowLevelsContext::LoadGame)
		SLoadingScreenWidget::Hide();

	// The batch that started this transition (whichever entry point it was) is now
	// fully shown — safe to let a new transition begin.
	bIsLevelTransitionActive = false;
}

void ASceneManager::OnLevelHidden()
{
	IncrementHiddenCount();
}

void ASceneManager::IncrementHiddenCount()
{
	if (CurrentLevelsHidden >= TotalLevelsToHide)
		return;

	++CurrentLevelsHidden;

	if (CurrentLevelsHidden < TotalLevelsToHide)
		return;

	OnAllLevelsHidden();
}

void ASceneManager::OnAllLevelsHidden()
{
	// All old levels are hidden. Now show the restore set
	if (PendingRestoreLevels.Num() > 0)
	{
		TArray<FName> LevelsToShow = MoveTemp(PendingRestoreLevels);
		ShowStreamingLevels(LevelsToShow, PendingShowContext);
		RuntimeVisibleLevels = LevelsToShow;
	}
	else
	{
		// Nothing to show – trigger completion directly
		ActiveShowContext = PendingShowContext;
		OnAllLevelsShown();
	}
}

void ASceneManager::OnAllLevelsReady()
{
	bLevelsReady = true;
	TryInitializeScene();
}

void ASceneManager::OnPostLoad_Implementation()
{
	bSaveLoaded = true;
	TryInitializeScene();
	
}

void ASceneManager::TryInitializeScene()
{
	if (!bLevelsReady || !bSaveLoaded || bSceneInitialized)
	{
		return;
	}

	bSceneInitialized = true;

	bool bHasSaveData = StreamingLevelsLoadedFromSave.Num() > 0 || !PlayerStartLocation.IsZero();

	if (bHasSaveData)
	{
		// Show the saved levels or defaults if no saved levels
		if (StreamingLevelsLoadedFromSave.Num() > 0)
		{
			ShowStreamingLevels(StreamingLevelsLoadedFromSave, EShowLevelsContext::InitialLoad);
			RuntimeVisibleLevels = StreamingLevelsLoadedFromSave;
		}
		else
		{
			// We have a saved position but no saved levels – fall back to defaults
			ShowStreamingLevels(StartingStreamingLevelsToShow, EShowLevelsContext::InitialLoad);
			RuntimeVisibleLevels = StartingStreamingLevelsToShow;
		}

		// Spawn the character and set its location to the saved one
		//SpawnCharacter();
		
	}
	else
	{
		// No save data, show the default levels and spawn the character at the PlayerStart
		ShowStreamingLevels(StartingStreamingLevelsToShow, EShowLevelsContext::InitialLoad);
		// Initialize runtime tracking from defaults
		RuntimeVisibleLevels = StartingStreamingLevelsToShow;

		StreamingLevelsLoadedFromSave = StartingStreamingLevelsToShow;
		CurrentCheckpointPriority = 0; 
       
		if (PlayerStart)
		{
			PlayerStartLocation = PlayerStart->GetActorLocation();
			PlayerStartRotation = PlayerStart->GetActorRotation();
		}
		
		TrySaveGame();

		// Snapshot the initial state as baseline for death-reset
		if (UStillHearGameInstance* InitGI = GetGameInstance<UStillHearGameInstance>())
		{
			InitGI->OnCheckpointSnapshot.Broadcast();
		}
	}

}


void ASceneManager::LoadLevelsStreaming(const TArray<FName>& Levels)
{
	for (int32 i = 0; i < Levels.Num(); ++i)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.UUID = GetUniqueID() + i;

		UGameplayStatics::LoadStreamLevel(
			this,
			Levels[i],
			false,
			false,
			LatentInfo);
	}
}

void ASceneManager::UnloadLevelsStreaming(const TArray<FName>& Levels)
{
	for (int32 i = 0; i < Levels.Num(); ++i)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.UUID = GetUniqueID() + i;

		UGameplayStatics::UnloadStreamLevel(
			this,
			Levels[i],
			LatentInfo,
			false);
	}
}

void ASceneManager::ShowStreamingLevels(const TArray<FName>& Levels, EShowLevelsContext Context)
{
	ActiveShowContext = Context;
	TotalLevelsToShow = Levels.Num();
	CurrentLevelsShown = 0;

	// Edge case: nothing to show → complete immediately
	if (TotalLevelsToShow == 0)
	{
		OnAllLevelsShown();
		return;
	}

	for (const FName& Level : Levels)
	{
		ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
		if (!StreamingLevel)
		{
			// Count missing levels as already shown so we don't block forever
			IncrementShownCount();
			continue;
		}

		// If the level is already visible, OnLevelShown won't fire again,
		// so we count it immediately.
		if (StreamingLevel->IsLevelVisible())
		{
			IncrementShownCount();
			continue;
		}

		// Level needs to become visible: bind the delegate and request visibility
		StreamingLevel->OnLevelShown.AddUniqueDynamic(this, &ASceneManager::OnLevelShown);
		StreamingLevel->SetShouldBeVisible(true);
	}
}

void ASceneManager::HideStreamingLevels(const TArray<FName>& Levels)
{
	for (const FName& Level : Levels)
	{
		ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
		if (StreamingLevel)
		{
			StreamingLevel->SetShouldBeVisible(false);
		}
	}
}

void ASceneManager::SpawnCharacter()
{
	if (!PlayerCharacterClass)
	{
		return;
	}

	FVector SpawnLocation;
	FRotator SpawnRotation;
	
	if (PlayerStartLocation != FVector::ZeroVector || !PlayerStartRotation.IsZero())
	{
		SpawnLocation = PlayerStartLocation;
		SpawnRotation = PlayerStartRotation;
	}
	else if (PlayerStart)
	{
		SpawnLocation = PlayerStart->GetActorLocation();
		SpawnRotation = PlayerStart->GetActorRotation();
	}
	else
	{
		SpawnLocation = FVector::ZeroVector;
		SpawnRotation = FRotator::ZeroRotator;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlayerCharacter = GetWorld()->SpawnActor<ACharacter>(
		PlayerCharacterClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (PlayerCharacter)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->Possess(PlayerCharacter);
		}

		if (AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(PlayerCharacter))
		{
			MainCharacter->OnInitializedFinished.AddUniqueDynamic(this, &ASceneManager::OnCharacterInitialized);
			MainCharacter->OnDeath.AddUniqueDynamic(this, &ASceneManager::OnCharacterDeath);
		}
	}
}

void ASceneManager::OnCharacterInitialized()
{
}

void ASceneManager::OnCharacterDeath()
{
	// Reset time dilation immediately on death to ensure respawn timers run at normal speed
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
		if (UTimeManagementSubsystem* TimeSubsystem = World->GetSubsystem<UTimeManagementSubsystem>())
		{
			TimeSubsystem->ResetTimeDilation();
		}
	}

	// Disable input immediately
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		PC->DisableInput(PC);

	// Read initial delay from settings
	float InitialDelay = 2.0f;
	if (const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>())
		InitialDelay = Settings->DeathScreenInitialDelay;

	if (InitialDelay > 0.0f)
	{
		// Delay before showing the death screen
		FTimerHandle InitialTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(InitialTimerHandle, this, &ASceneManager::ShowDeathScreen, InitialDelay, false);
	}
	else
	{
		ShowDeathScreen();
	}
}

void ASceneManager::ShowDeathScreen()
{
	// Show the death screen
	SLoadingScreenWidget::Show(true);

	// Read duration from settings
	float DurationDelay = 3.0f;
	if (const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>())
		DurationDelay = Settings->DeathScreenDuration;

	// Delay the respawn process
	FTimerHandle RespawnTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ASceneManager::RestoreStreamingLevels, DurationDelay, false);
}

void ASceneManager::RestoreStreamingLevels()
{
	bIsLevelTransitionActive = true;

	// Determine which levels to restore (saved state or defaults)
	const TArray<FName>& LevelsToRestore = (StreamingLevelsLoadedFromSave.Num() > 0)
		? StreamingLevelsLoadedFromSave
		: StartingStreamingLevelsToShow;

	// Compute which currently-visible levels must be HIDDEN
	// (those in RuntimeVisibleLevels but NOT in LevelsToRestore)
	TArray<FName> LevelsToHide;
	for (const FName& Level : RuntimeVisibleLevels)
	{
		if (!LevelsToRestore.Contains(Level))
			LevelsToHide.Add(Level);
	}

	// Store which levels we'll need to SHOW after the hide completes
	// (those in LevelsToRestore but NOT currently visible, plus those that
	//  we are about to hide and will need to re-show)
	PendingRestoreLevels = LevelsToRestore;

	// Clear runtime tracking — it will be rebuilt when show completes
	RuntimeVisibleLevels.Empty();

	if (LevelsToHide.Num() > 0)
	{
		PendingShowContext = EShowLevelsContext::Respawn;
		// Async hide: when all are hidden, OnAllLevelsHidden will show PendingRestoreLevels
		TotalLevelsToHide = LevelsToHide.Num();
		CurrentLevelsHidden = 0;

		for (const FName& Level : LevelsToHide)
		{
			ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
			if (!StreamingLevel)
			{
				IncrementHiddenCount();
				continue;
			}

			if (!StreamingLevel->IsLevelVisible())
			{
				IncrementHiddenCount();
				continue;
			}

			StreamingLevel->OnLevelHidden.AddUniqueDynamic(this, &ASceneManager::OnLevelHidden);
			StreamingLevel->SetShouldBeVisible(false);
		}
	}
	else
	{
		// Nothing to hide — go straight to showing the restore set
		ShowStreamingLevels(PendingRestoreLevels, EShowLevelsContext::Respawn);
		RuntimeVisibleLevels = MoveTemp(PendingRestoreLevels);
	}
}

void ASceneManager::ResetCharacter()
{
	ResetActors();
	
	if (!PlayerCharacter)
	{
		return;
	}

	FVector ResetLocation;
	FRotator ResetRotation;

	if (PlayerStartLocation != FVector::ZeroVector || !PlayerStartRotation.IsZero())
	{
		ResetLocation = PlayerStartLocation;
		ResetRotation = PlayerStartRotation;
	}
	else if (PlayerStart)
	{
		ResetLocation = PlayerStart->GetActorLocation();
		ResetRotation = PlayerStart->GetActorRotation();
	}
	else
	{
		return;
	}

	PlayerCharacter->SetActorLocationAndRotation(ResetLocation, ResetRotation,false, nullptr, ETeleportType::None);

	// Revive the character: undo ragdoll, re-enable movement and collisions
	if (AStillHearMainCharacter* MainCharacter = Cast<AStillHearMainCharacter>(PlayerCharacter))
	{
		MainCharacter->Revive();
	}
}

void ASceneManager::ResetActors() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			GI->OnRequestWorldReset.Broadcast();
		}
	}
}

void ASceneManager::TrySaveGame()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
		return;

	USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>();
	if (SaveSubsystem)
	{
		if (SaveSubsystem->GetCurrentSlotSave() > 0)
		{
			SaveSubsystem->RequestSaveSlotAsync(SaveSubsystem->GetCurrentSlotSave());
		}
	}
}

ACameraVolume* ASceneManager::FindCameraVolumeAtLocation(const FVector& Location) const
{
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraVolume::StaticClass(), FoundVolumes);

	ACameraVolume* Best = nullptr;
	int32 BestPriority = INT32_MIN;

	for (AActor* Actor : FoundVolumes)
	{
		ACameraVolume* Vol = Cast<ACameraVolume>(Actor);
		if (Vol && Vol->ContainsPoint(Location) && Vol->GetPriority() > BestPriority)
		{
			Best = Vol;
			BestPriority = Vol->GetPriority();
		}
	}

	return Best;
}

void ASceneManager::SetupMenuView(bool bHasSaveData)
{
	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
		return;

	ACameraVolume* StartingVolume = nullptr;

	if (bHasSaveData)
	{
		StartingVolume = FindCameraVolumeAtLocation(PlayerStartLocation);
	}

	if (!StartingVolume)
	{
		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraVolume::StaticClass(), DefaultMenuCameraTag, FoundVolumes);
		if (FoundVolumes.Num() > 0)
		{
			StartingVolume = Cast<ACameraVolume>(FoundVolumes[0]);
		}
	}

	if (!StartingVolume)
	{
		TArray<AActor*> FoundVolumes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraVolume::StaticClass(), FoundVolumes);
		if (FoundVolumes.Num() > 0)
		{
			StartingVolume = Cast<ACameraVolume>(FoundVolumes[0]);
		}
	}

	if (StartingVolume)
	{
		StartingVolume->ResetToDefaultTransform();
		PC->ChangeCamera(StartingVolume, nullptr);
	}
}

void ASceneManager::TransitionToGameplay()
{
	if (!bIsInMenuMode)
		return;

	bIsInMenuMode = false;

	SpawnCharacter();

	if (!PlayerCharacter)
		return;

	AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
		return;

	PC->TransitionToPossession(PlayerCharacter);

	PC->SetInputMode(FInputModeGameOnly());

	StartFlowRoot();
}

void ASceneManager::StartFlowRoot()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
		{
			if (RootFlowAsset)
			{
				bool bRestored = false;
				if (!SavedRootFlowInstanceName.IsEmpty())
				{
					FlowSubsystem->LoadRootFlow(this, RootFlowAsset, SavedRootFlowInstanceName, false);
					bRestored = !FlowSubsystem->GetRootInstancesByOwner(this).IsEmpty();
				}

				// LoadRootFlow silently no-ops on a stale/mismatched saved name; fall back to a
				// fresh flow so we're never left without one, and re-sync the saved name to it.
				if (!bRestored)
				{
					UFlowAsset* FlowInstance = FlowSubsystem->CreateRootFlow(this, RootFlowAsset, false);
					if (FlowInstance)
					{
						SavedRootFlowInstanceName = FlowInstance->GetName();
						FlowInstance->StartFlow();
					}
				}
			}
		}
	}
}

bool ASceneManager::LoadSlotAndTransition(const int32 SlotIndex, const bool bAllowFullReload)
{
	// Mask the map reload(s) and level streaming behind a loading screen; Show() no-ops if
	// already active, so it stays up across the chained New Game / Load Game hand-offs.
	SLoadingScreenWidget::Show();

	if (bIsLevelTransitionActive)
	{
		// Another transition is still hiding/showing its levels
		TWeakObjectPtr WeakThis(this);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([WeakThis, SlotIndex, bAllowFullReload]()
			{
				if (ASceneManager* SafeThis = WeakThis.Get())
					SafeThis->LoadSlotAndTransition(SlotIndex, bAllowFullReload);
			});
		}
		return true;
	}

	const UGameInstance* GI = GetGameInstance();
	if (!GI)
		return false;

	USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
	if (!SaveSub)
		return false;

	const AStillHearPlayerController* PC = Cast<AStillHearPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC && PC->PlayerCameraManager)
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);

	// Reset so the deserialization from save can restore the correct value (or leave it empty for a fresh start)
	// Without this, a stale name from an aborted session would cause LoadRootFlow to look for a non-existent instance
	SavedRootFlowInstanceName = TEXT("");

	bool bLoaded = false;
	const FString SlotName = FString::Printf(TEXT("Save_Slot%d"), SlotIndex);
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (bAllowFullReload)
		{
			// Same full-reload fix as New Game: get a pristine world first, then hand off
			// back here (bAllowFullReload=false) once BeginPlay has settled to actually load.
			SaveSub->ResetGameSession();

			if (UStillHearGameInstance* StillHearGI = GetWorld()->GetGameInstance<UStillHearGameInstance>())
				StillHearGI->PendingLoadGameSlotIndex = SlotIndex;

			UGameplayStatics::OpenLevel(this, SaveSub->GetStableMapKey(GetWorld()));
			return true;
		}

		bLoaded = SaveSub->LoadFromSlot(SlotIndex, 0);
	}
	else
	{
		// Genuine New Game
		// Fully reload the persistent map
		SaveSub->ResetGameSession();

		USaveGameObject* NewSave = Cast<USaveGameObject>(UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass()));
		if (!NewSave)
			return false;

		NewSave->SaveVersion = 1;
		NewSave->SlotName = FString::Printf(TEXT("Save_Slot%d"), SlotIndex);
		NewSave->SaveDate = FDateTime::UtcNow();
		NewSave->LastLevelName = SaveSub->GetStableMapKey(GetWorld());

		const bool bSaved = UGameplayStatics::SaveGameToSlot(NewSave, NewSave->SlotName, 0);
		if (!bSaved)
			return false;

		// Remember which slot to jump into once the reloaded map's menu has finished
		if (UStillHearGameInstance* StillHearGI = GetWorld()->GetGameInstance<UStillHearGameInstance>())
		{
			StillHearGI->PendingNewGameSlotIndex = SlotIndex;
			StillHearGI->bIsSpawningNewGame = true;
		}

		UGameplayStatics::OpenLevel(this, SaveSub->GetStableMapKey(GetWorld()));
		return true;
	}

	if (!bLoaded)
		return false;

	bIsLevelTransitionActive = true;

	if (PC)
	{
		if (UUISubsystem* UISubsystem = PC->GetLocalPlayer()->GetSubsystem<UUISubsystem>())
			UISubsystem->ClearLayer(TAG_UI_Layer_Menu);
	}

	const TArray<FName>& LevelsToRestore = (StreamingLevelsLoadedFromSave.Num() > 0)
		? StreamingLevelsLoadedFromSave
		: StartingStreamingLevelsToShow;

	TArray<FName> LevelsToHide;
	for (const FName& Level : RuntimeVisibleLevels)
	{
		if (!LevelsToRestore.Contains(Level))
			LevelsToHide.Add(Level);
	}

	PendingRestoreLevels = LevelsToRestore;
	RuntimeVisibleLevels.Empty();

	if (LevelsToHide.Num() > 0)
	{
		PendingShowContext = EShowLevelsContext::LoadGame;
		TotalLevelsToHide = LevelsToHide.Num();
		CurrentLevelsHidden = 0;

		for (const FName& Level : LevelsToHide)
		{
			ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
			if (!StreamingLevel)
			{
				IncrementHiddenCount();
				continue;
			}

			if (!StreamingLevel->IsLevelVisible())
			{
				IncrementHiddenCount();
				continue;
			}

			StreamingLevel->OnLevelHidden.AddUniqueDynamic(this, &ASceneManager::OnLevelHidden);
			StreamingLevel->SetShouldBeVisible(false);
		}
	}
	else
	{
		ShowStreamingLevels(PendingRestoreLevels, EShowLevelsContext::LoadGame);
		RuntimeVisibleLevels = MoveTemp(PendingRestoreLevels);
	}

	return true;
}

void ASceneManager::RefreshMenuBackground()
{
	if (!bIsInMenuMode)
		return;

	UGameInstance* GI = GetGameInstance();
	if (!GI)
		return;

	USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
	if (!SaveSub)
		return;

	const int32 LatestSlot = SaveSub->GetLatestSaveSlotIndex();

	if (LatestSlot <= 0)
	{
		ResetMenuToDefault();
		return;
	}

	const bool bLoaded = SaveSub->LoadFromSlot(LatestSlot, 0, false);

	if (!bLoaded)
		return;

	bIsLevelTransitionActive = true;

	SetupMenuView(true);

	const TArray<FName>& LevelsToRestore = (StreamingLevelsLoadedFromSave.Num() > 0)
		? StreamingLevelsLoadedFromSave
		: StartingStreamingLevelsToShow;

	TArray<FName> LevelsToHide;
	for (const FName& Level : RuntimeVisibleLevels)
	{
		if (!LevelsToRestore.Contains(Level))
			LevelsToHide.Add(Level);
	}

	PendingRestoreLevels = LevelsToRestore;
	RuntimeVisibleLevels.Empty();

	if (LevelsToHide.Num() > 0)
	{
		PendingShowContext = EShowLevelsContext::InitialLoad;
		TotalLevelsToHide = LevelsToHide.Num();
		CurrentLevelsHidden = 0;

		for (const FName& Level : LevelsToHide)
		{
			ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
			if (StreamingLevel && StreamingLevel->IsLevelVisible())
			{
				StreamingLevel->OnLevelHidden.AddUniqueDynamic(this, &ASceneManager::OnLevelHidden);
				StreamingLevel->SetShouldBeVisible(false);
			}
			else
			{
				IncrementHiddenCount();
			}
		}
	}
	else
	{
		ShowStreamingLevels(PendingRestoreLevels, EShowLevelsContext::InitialLoad);
		RuntimeVisibleLevels = MoveTemp(PendingRestoreLevels);
	}
}

void ASceneManager::ResetMenuToDefault()
{
	if (!bIsInMenuMode)
		return;

	bIsLevelTransitionActive = true;

	PlayerStartLocation = FVector::ZeroVector;
	PlayerStartRotation = FRotator::ZeroRotator;
	StreamingLevelsLoadedFromSave.Empty();
	CurrentCheckpointPriority = -1;

	SetupMenuView(false);

	const TArray<FName>& LevelsToRestore = StartingStreamingLevelsToShow;

	TArray<FName> LevelsToHide;
	for (const FName& Level : RuntimeVisibleLevels)
	{
		if (!LevelsToRestore.Contains(Level))
			LevelsToHide.Add(Level);
	}

	PendingRestoreLevels = LevelsToRestore;
	RuntimeVisibleLevels.Empty();

	if (LevelsToHide.Num() > 0)
	{
		PendingShowContext = EShowLevelsContext::InitialLoad;
		TotalLevelsToHide = LevelsToHide.Num();
		CurrentLevelsHidden = 0;

		for (const FName& Level : LevelsToHide)
		{
			ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, Level);
			if (StreamingLevel && StreamingLevel->IsLevelVisible())
			{
				StreamingLevel->OnLevelHidden.AddUniqueDynamic(this, &ASceneManager::OnLevelHidden);
				StreamingLevel->SetShouldBeVisible(false);
			}
			else
			{
				IncrementHiddenCount();
			}
		}
	}
	else
	{
		ShowStreamingLevels(PendingRestoreLevels, EShowLevelsContext::InitialLoad);
		RuntimeVisibleLevels = MoveTemp(PendingRestoreLevels);
	}
}

void ASceneManager::ReturnToMainMenu()
{
	bIsInMenuMode = true;

	if (PlayerCharacter)
	{
		PlayerCharacter->Destroy();
		PlayerCharacter = nullptr;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
		{
			SaveSub->ResetGameSession();
		}
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	RefreshMenuBackground();
}