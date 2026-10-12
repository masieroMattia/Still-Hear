#include "Flow/CheckpointBase.h"

#include "Engine/GameInstance.h"
#include "SaveSystem/SaveSubsystem.h"

// Sets default values
ACheckpointBase::ACheckpointBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PlayerNewStartPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerNewStartPoint"));
	RootComponent = PlayerNewStartPoint;
	PlayerNewStartPoint->SetArrowColor(FLinearColor::Green);
	PlayerNewStartPoint->SetArrowSize(2.0f);
	PlayerNewStartPoint->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void ACheckpointBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACheckpointBase::OnCheckpointActivated()
{
	if (!SceneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckpointBase: SceneManager is not set!"));
		return;
	}

	// Always apply the visual transition (show/hide levels) when going forward
	// This is needed even for previously-activated checkpoints after a game restart
	bTraversedForward = true;

	if (bCallSaveOnActivate && SceneManager->CanActivateCheckpoint(CheckpointPriority))
	{
		// Tag the save with this checkpoint's ID before persisting
		if (!CheckpointId.IsNone())
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
					SaveSub->SetCurrentCheckpoint(CheckpointId);
			}
		}

		// ── Saving checkpoint: persist everything to disk ──
		SceneManager->UpdateSavedState(
			LevelsToShow,
			LevelsToHide,
			PlayerNewStartPoint->GetComponentLocation(),
			PlayerNewStartPoint->GetComponentRotation(),
			CheckpointPriority
		);
	}
	else
	{
		// ── Non-saving, or going backwards through a saving checkpoint ──
		// Only change what the player sees; the save data stays untouched.
		SceneManager->UpdateRuntimeVisibility(LevelsToShow, LevelsToHide);
	}
}

void ACheckpointBase::OnCheckpointReversed()
{
	if (!bTraversedForward)
	{
		return;
	}

	if (!SceneManager)
	{
		return;
	}

	bTraversedForward = false;

	// Reverse: show what was hidden, hide what was shown
	SceneManager->UpdateRuntimeVisibility(LevelsToHide, LevelsToShow);
}

void ACheckpointBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}