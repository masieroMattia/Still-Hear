#include "Interactions/Actors/TargetableDrivableObj.h"

#include "Interactions/Components/InteractableShakeComponent.h"

#pragma region CONSTRUCTOR
ATargetableDrivableObj::ATargetableDrivableObj()
{
	TargetNiagaraComponent = CreateDefaultSubobject<UTargetMarkerNiagaraComponent>("TargetNiagaraComponent");
	TargetNiagaraComponent->SetupAttachment(RootComponent);
	TargetNiagaraComponent->SetAutoActivate(false);
	
	HitNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("HitNiagaraComponent");
	HitNiagaraComponent->SetupAttachment(RootComponent);
	HitNiagaraComponent->SetAutoActivate(false);
}
#pragma endregion
	
#pragma region OVERRIDE METHODS
void ATargetableDrivableObj::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache initial targetability to restore it on reset
	bCachedIsTargetable = bIsTargetable;
}

void ATargetableDrivableObj::Reset()
{
	Super::Reset();
	
	const bool bFromSnapshot = bHasTargetableCheckpointSnapshot;
	bIsTargetable = bFromSnapshot ? bCheckpointIsTargetable : bCachedIsTargetable;
	bWasHitInteraction = bFromSnapshot ? bCheckpointWasHitInteraction : false;
}

void ATargetableDrivableObj::ExecuteStartInteraction(const TObjectPtr<ACharacter> Interactor)
{
	// When bMoveOnHit is false, completely bypass ADrivableObj's timeline/movement logic
	if (bIsExecutingHit && !bMoveOnHit)
	{
		AInteractableObj::ExecuteStartInteraction(Interactor);

		for (const TObjectPtr LinkedObj : LinkedTriggeredObjs)
		{
			if (LinkedObj && LinkedObj->Implements<UInteractable>())
				LinkedObj->TriggerInteraction(this);
		}

		if (bAutoReset)
		{
			FTimerHandle AutoResetHandle;
			GetWorldTimerManager().SetTimer(AutoResetHandle, this, &ATargetableDrivableObj::StopHitTarget, AutoResetDelay, false);
		}

		return;
	}

	Super::ExecuteStartInteraction(Interactor);

	// Auto-reset if no movement curve is configured to prevent getting stuck
	if (bIsInteracting && bAutoReset && (!TransformCurve || DrivableParts.Num() == 0))
	{
		FTimerHandle AutoResetHandle;
		GetWorldTimerManager().SetTimer(AutoResetHandle, this, &ATargetableDrivableObj::StopHitTarget, AutoResetDelay, false);
	}
}

void ATargetableDrivableObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	Super::EndInteraction(Interactor);
	bWasHitInteraction = false;
}

void ATargetableDrivableObj::SaveCheckpointState()
{
	Super::SaveCheckpointState();
	bHasTargetableCheckpointSnapshot = true;
	bCheckpointIsTargetable = bIsTargetable;
	bCheckpointWasHitInteraction = bWasHitInteraction;
}

void ATargetableDrivableObj::ClearCheckpointState()
{
	Super::ClearCheckpointState();
	bHasTargetableCheckpointSnapshot = false;
}
#pragma endregion

#pragma region INTERFACE METHODS
void ATargetableDrivableObj::SetTargeted()
{
	if (bIsTargetable && TargetNiagaraComponent)
		TargetNiagaraComponent->Activate();

	if (ShakeComponent)
		ShakeComponent->SetInRange(true);
}

void ATargetableDrivableObj::SetUntargeted()
{
	if (TargetNiagaraComponent)
		TargetNiagaraComponent->Deactivate();

	if (ShakeComponent)
		ShakeComponent->SetInRange(false);
}

void ATargetableDrivableObj::HitTarget(const int32 DeflectionsCount)
{
	// Let subclasses decide whether this hit count satisfies their activation condition
	if (ShouldActivateOnHit(DeflectionsCount))
	{
		bWasHitInteraction = true;
		bIsExecutingHit = true;
		StartInteraction();
		bIsExecutingHit = false;
	}

	OnTargetHit.Broadcast(DeflectionsCount);
	
	if (HitNiagaraComponent)
		HitNiagaraComponent->Activate(true);
	
	if (bTargetOnce)
		bIsTargetable = false;
}

void ATargetableDrivableObj::StopHitTarget()
{
	EndInteraction();
}

bool ATargetableDrivableObj::IsTargetable() const
{
	return bIsTargetable;
}
#pragma endregion
