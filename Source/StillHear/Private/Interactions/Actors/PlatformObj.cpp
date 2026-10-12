#include "Interactions/Actors/PlatformObj.h"

#include "StillHearGameInstance.h"

#pragma region OVERRIDE METHODS
void APlatformObj::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex, const bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (!OtherActor || OtherActor == this)
		return;

	// In OneShot mode, ignore overlaps once already triggered
	if (PlateMode == EPressurePlateMode::OneShot && bPlateTriggered)
		return;

	if (!IsValidPlateActivator(OtherActor, OtherComp))
		return;

	bPlateOccupied = true;
	CurrentPlateActivator = OtherActor;

	if (InteractNiagaraComponent)
		InteractNiagaraComponent->Activate();

	// Instant activation or start countdown
	if (PlateActivationTime <= 0.0f)
	{
		ActivatePlate();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			PlateActivationTimerHandle,
			this,
			&APlatformObj::OnPlateActivationTimerExpired,
			PlateActivationTime,
			false
		);
	}
}

void APlatformObj::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (!OtherActor || OtherActor != CurrentPlateActivator)
		return;

	bPlateOccupied = false;
	CurrentPlateActivator = nullptr;

	// Cancel any pending activation countdown
	GetWorldTimerManager().ClearTimer(PlateActivationTimerHandle);

	if (InteractNiagaraComponent)
		InteractNiagaraComponent->Deactivate();

	// In Persistent mode, undo the trigger when the activator leaves
	if (PlateMode == EPressurePlateMode::Persistent && bPlateTriggered)
	{
		DeactivatePlate();
	}
}

void APlatformObj::Reset()
{
	bool bForceNewGame = false;
	if (const UWorld* World = GetWorld())
	{
		if (const UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			bForceNewGame = GI->bIsNewGameResetting;
		}
	}

	if (!bResetObj && !bForceNewGame)
		return;

	Super::Reset();

	const bool bFromSnapshot = bHasPlatformCheckpointSnapshot;
	bPlateTriggered = bFromSnapshot ? bCheckpointPlateTriggered : false;

	bPlateOccupied = false;
	CurrentPlateActivator = nullptr;
	GetWorldTimerManager().ClearTimer(PlateActivationTimerHandle);

	if (InteractNiagaraComponent)
		InteractNiagaraComponent->Deactivate();
}

void APlatformObj::SaveCheckpointState()
{
	Super::SaveCheckpointState();
	bHasPlatformCheckpointSnapshot = true;
	bCheckpointPlateTriggered = bPlateTriggered;
}

void APlatformObj::ClearCheckpointState()
{
	Super::ClearCheckpointState();
	bHasPlatformCheckpointSnapshot = false;
}
#pragma endregion

#pragma region METHODS
bool APlatformObj::IsValidPlateActivator(AActor* Actor, UPrimitiveComponent* OtherComp) const
{
	if (!Actor)
		return false;

	// If a specific list of actors is required, only they can activate the plate
	if (RequiredActors.Num() > 0)
	{
		return RequiredActors.Contains(Actor);
	}

	// No specific actor filter — accept anything that overlaps (channel filtering is already handled by the SphereComponent's CollisionChannels)
	return true;
}

void APlatformObj::ActivatePlate()
{
	if (bPlateTriggered)
		return;

	bPlateTriggered = true;
	
	StartInteraction();
}

void APlatformObj::DeactivatePlate()
{
	if (!bPlateTriggered)
		return;

	bPlateTriggered = false;

	StartInteraction();
}

void APlatformObj::OnPlateActivationTimerExpired()
{
	// Safety: the activator might have left during the countdown
	if (!bPlateOccupied)
		return;

	ActivatePlate();
}
#pragma endregion