#include "Interactions/Actors/ResonancePuzzleManager.h"

#include "Interfaces/Restorable.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Components/AudioComponent.h"
#include "Interactions/Actors/RailMoveObj.h"
#include "Interactions/Actors/InteractableObj.h"
#include "Interactions/Actors/ChaosResonanceObj.h"

// UPuzzleBellListener Implementation
void UPuzzleBellListener::OnInteractableStarted()
{
	if (Manager.IsValid() && BellActor.IsValid())
	{
		Manager->HandleBellResonance(BellActor.Get());
	}
}

void UPuzzleBellListener::OnChaosTriggered(AActor* Triggerer)
{
	if (Manager.IsValid() && BellActor.IsValid())
	{
		Manager->HandleBellResonance(BellActor.Get());
	}
}

// AResonancePuzzleManager Implementation
AResonancePuzzleManager::AResonancePuzzleManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and set root component so the actor can be placed and moved in the world (supporting spatialized audio)
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	// Pre-allocate audio components to avoid spawning/destroying UAudioComponents at runtime
	MetronomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MetronomeAudioComponent"));
	MetronomeAudioComponent->SetupAttachment(RootComponent);
	MetronomeAudioComponent->bAutoActivate = false;

	FeedbackAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FeedbackAudioComponent"));
	FeedbackAudioComponent->SetupAttachment(RootComponent);
	FeedbackAudioComponent->bAutoActivate = false;

	// Create Action Area trigger box
	ActionAreaCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("ActionAreaCollider"));
	ActionAreaCollider->SetupAttachment(RootComponent);
	ActionAreaCollider->SetCollisionProfileName(TEXT("Trigger"));
}

void AResonancePuzzleManager::BeginPlay()
{
	Super::BeginPlay();

	ResolveAllReferences();
	CreateDynamicMaterials();
	BindToBells();

	// Bind to overlap events for the action area
	if (ActionAreaCollider)
	{
		ActionAreaCollider->OnComponentBeginOverlap.AddDynamic(this, &AResonancePuzzleManager::OnActionAreaBeginOverlap);
		ActionAreaCollider->OnComponentEndOverlap.AddDynamic(this, &AResonancePuzzleManager::OnActionAreaEndOverlap);
	}

	// Wait for the next tick so overlaps are fully initialized
	GetWorldTimerManager().SetTimerForNextTick(this, &AResonancePuzzleManager::CheckInitialBellsAndStart);
}

void AResonancePuzzleManager::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMetronome(DeltaTime);
	UpdateGemMaterials(DeltaTime);
	UpdatePatternStepAnimations();
	UpdateSuccessGlow(DeltaTime);
}

UStaticMeshComponent* AResonancePuzzleManager::ResolveComponentRef(const FComponentReference& Ref) const
{
	AActor* ResolveActor = Ref.OtherActor.Get();
	if (!ResolveActor)
	{
		ResolveActor = const_cast<AResonancePuzzleManager*>(this);
	}
	return Cast<UStaticMeshComponent>(Ref.GetComponent(ResolveActor));
}

void AResonancePuzzleManager::ResolveAllReferences()
{
	// Resolve Metronome Gems
	ResolvedMetronomeGems.Empty();
	for (const FMetronomeStepConfig& Step : MetronomeSteps)
	{
		for (const FComponentReference& Ref : Step.Gems)
		{
			if (UStaticMeshComponent* GemMesh = ResolveComponentRef(Ref))
			{
				ResolvedMetronomeGems.AddUnique(GemMesh);
			}
		}
	}

	// Resolve Puzzle Steps, Visual Meshes, and Bell Actors
	ResolvedPuzzleSteps.Empty();
	ResolvedPatternStepMeshes.Empty();
	ResolvedBells.Empty();

	for (const FPuzzleStepConfig& StepConfig : PuzzleSteps)
	{
		FResolvedPuzzleStep ResolvedStep;
		ResolvedStep.bIsPauseStep = StepConfig.bIsPauseStep;

		// Resolve visual mesh
		if (UStaticMeshComponent* StepMesh = ResolveComponentRef(StepConfig.StepVisualMesh))
		{
			ResolvedStep.VisualMesh = StepMesh;
			ResolvedPatternStepMeshes.AddUnique(StepMesh);
		}

		// Resolve expected bell actor if it is not a pause step
		if (!StepConfig.bIsPauseStep)
		{
			AActor* BellActor = StepConfig.ExpectedBell.OtherActor.Get();
			if (BellActor)
			{
				ResolvedStep.ExpectedBellActor = BellActor;
				ResolvedBells.AddUnique(BellActor);
			}
		}

		ResolvedPuzzleSteps.Add(ResolvedStep);
	}

	// Resolve Success Glow Components
	ResolvedSuccessGlowComponents.Empty();
	for (const FComponentReference& Ref : SuccessGlowComponents)
	{
		if (UStaticMeshComponent* GlowMesh = ResolveComponentRef(Ref))
		{
			ResolvedSuccessGlowComponents.AddUnique(GlowMesh);
		}
	}
}

void AResonancePuzzleManager::CreateDynamicMaterials()
{
	MetronomeGemMIDs.Empty();
	MetronomeGemIntensities.Empty();
	
	PatternStepMIDs.Empty();
	PatternStepIntensities.Empty();
	PatternStepColors.Empty();

	SuccessGlowMIDs.Empty();
	SuccessGlowIntensities.Empty();

	// Create MIDs for Metronome Gems
	for (UStaticMeshComponent* GemMesh : ResolvedMetronomeGems)
	{
		if (!GemMesh) continue;
		
		UMaterialInterface* BaseMaterial = GemMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (MID)
			{
				GemMesh->SetMaterial(0, MID);
				MetronomeGemMIDs.Add(GemMesh, MID);
				// Initialize to off
				MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				MetronomeGemIntensities.Add(GemMesh, 0.0f);
			}
		}
	}

	// Create MIDs for Pattern Step Meshes
	for (UStaticMeshComponent* StepMesh : ResolvedPatternStepMeshes)
	{
		if (!StepMesh) continue;

		UMaterialInterface* BaseMaterial = StepMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (MID)
			{
				StepMesh->SetMaterial(0, MID);
				PatternStepMIDs.Add(StepMesh, MID);
				// Initialize to inactive color and off
				MID->SetVectorParameterValue(EmissiveColorParamName, InactiveColor);
				MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				PatternStepIntensities.Add(StepMesh, 0.0f);
				PatternStepColors.Add(StepMesh, InactiveColor);
			}
		}
	}

	// Create MIDs for Success Glow Components
	for (UStaticMeshComponent* GlowMesh : ResolvedSuccessGlowComponents)
	{
		if (!GlowMesh) continue;

		UMaterialInterface* BaseMaterial = GlowMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (MID)
			{
				GlowMesh->SetMaterial(0, MID);
				SuccessGlowMIDs.Add(GlowMesh, MID);
				// Initialize to SuccessColor and off
				MID->SetVectorParameterValue(EmissiveColorParamName, SuccessColor);
				MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				SuccessGlowIntensities.Add(GlowMesh, 0.0f);
			}
		}
	}
}

void AResonancePuzzleManager::BindToBells()
{
	BellListeners.Empty();

	for (AActor* BellActor : ResolvedBells)
	{
		if (!BellActor) continue;

		UPuzzleBellListener* Listener = NewObject<UPuzzleBellListener>(this);
		Listener->BellActor = BellActor;
		Listener->Manager = this;

		if (AInteractableObj* InteractableObj = Cast<AInteractableObj>(BellActor))
		{
			InteractableObj->OnInteractionStarted.AddUniqueDynamic(Listener, &UPuzzleBellListener::OnInteractableStarted);
			BellListeners.Add(Listener);
		}
		else if (AChaosResonanceObj* ChaosObj = Cast<AChaosResonanceObj>(BellActor))
		{
			ChaosObj->OnTriggeredBy.AddUniqueDynamic(Listener, &UPuzzleBellListener::OnChaosTriggered);
			BellListeners.Add(Listener);
		}
	}
}

void AResonancePuzzleManager::UpdateMetronome(const float DeltaTime)
{
	if (MetronomeSteps.Num() == 0 || !bPuzzleActive || bInErrorState || bPuzzleSolved)
	{
		return;
	}

	StepTimer += DeltaTime;
	const FMetronomeStepConfig& CurrentStep = MetronomeSteps[CurrentMetronomeStepIndex];

	if (StepTimer >= CurrentStep.Duration)
	{
		// Step transition
		const int32 OldStep = CurrentMetronomeStepIndex;
		CurrentMetronomeStepIndex = (CurrentMetronomeStepIndex + 1) % MetronomeSteps.Num();
		StepTimer = 0.0f;

		OnMetronomeStepTransition(OldStep, CurrentMetronomeStepIndex);
	}
}

void AResonancePuzzleManager::OnMetronomeStepTransition(int32 OldStepIndex, int32 NewStepIndex)
{
	if (MetronomeSteps.Num() == 0) return;

	const FMetronomeStepConfig& OldStep = MetronomeSteps[OldStepIndex];
	const FMetronomeStepConfig& NewStep = MetronomeSteps[NewStepIndex];

	// Play new step sound using optimized audio component
	if (NewStep.StepSound && MetronomeAudioComponent)
	{
		MetronomeAudioComponent->SetSound(NewStep.StepSound);
		MetronomeAudioComponent->Play();
	}

	// Transitioning OUT of a resonance window
	if (OldStep.bIsResonanceWindow && !NewStep.bIsResonanceWindow)
	{
		if (CurrentPatternStepIndex < ResolvedPuzzleSteps.Num())
		{
			const FResolvedPuzzleStep& CurrentStep = ResolvedPuzzleSteps[CurrentPatternStepIndex];

			// Handle Wait/Pause steps
			if (CurrentStep.bIsPauseStep)
			{
				if (!bStepCompletedThisCycle)
				{
					// Player successfully stayed idle during the wait step!
					OnStepSucceeded(CurrentPatternStepIndex);
				}
			}
			// Handle Bell steps
			else
			{
				if (!bStepCompletedThisCycle)
				{
					// Player missed the resonance window entirely!
					TriggerFailure(true);
				}
			}
		}
	}

	// Transitioning INTO a resonance window
	if (NewStep.bIsResonanceWindow)
	{
		// Reset active step state
		bStepCompletedThisCycle = false;
	}
}

void AResonancePuzzleManager::UpdateGemMaterials(const float DeltaTime)
{
	if (MetronomeSteps.Num() == 0) return;

	const FMetronomeStepConfig& CurrentStep = MetronomeSteps[CurrentMetronomeStepIndex];

	for (UStaticMeshComponent* GemMesh : ResolvedMetronomeGems)
	{
		if (!GemMesh) continue;

		const TObjectPtr<UMaterialInstanceDynamic>* MIDPtr = MetronomeGemMIDs.Find(GemMesh);
		if (!MIDPtr || !*MIDPtr) 
			continue;

		// Determine target intensity
		float TargetIntensity = 0.0f;
		if (bPuzzleActive && !bInErrorState && !bPuzzleSolved && CurrentStep.Gems.ContainsByPredicate([GemMesh, this](const FComponentReference& Ref) {
			return ResolveComponentRef(Ref) == GemMesh;
		}))
		{
			TargetIntensity = ActiveGlowIntensity;
		}

		// Interpolate smoothly using cached value to avoid per-frame GetScalarParameterValue
		float CurrentVal = MetronomeGemIntensities.FindRef(GemMesh);
		const float NewVal = FMath::FInterpTo(CurrentVal, TargetIntensity, DeltaTime, GlowBlendSpeed);
		if (!FMath::IsNearlyEqual(CurrentVal, NewVal, 0.001f))
		{
			(*MIDPtr)->SetScalarParameterValue(EmissiveParamName, NewVal);
			MetronomeGemIntensities.Add(GemMesh, NewVal);
		}
	}
}

void AResonancePuzzleManager::UpdatePatternStepAnimations()
{
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (int32 i = 0; i < ResolvedPatternStepMeshes.Num(); ++i)
	{
		UStaticMeshComponent* StepMesh = ResolvedPatternStepMeshes[i];
		if (!StepMesh) continue;

		const TObjectPtr<UMaterialInstanceDynamic>* MIDPtr = PatternStepMIDs.Find(StepMesh);
		if (!MIDPtr || !*MIDPtr) continue;

		const EStepVisualState* VisualStatePtr = PatternStepVisualStates.Find(StepMesh);
		const EStepVisualState VisualState = VisualStatePtr ? *VisualStatePtr : EStepVisualState::Off;

		FLinearColor TargetColor = InactiveColor;
		float TargetIntensity = 0.0f;

		switch (VisualState)
		{
			case EStepVisualState::Flashing:
				{
					TargetColor = ActiveColor;
					// Sine wave animation between MinFlashGlowIntensity and ActiveGlowIntensity
					float SineVal = (FMath::Sin(TimeSeconds * FlashFrequency) + 1.0f) * 0.5f;
					TargetIntensity = FMath::Lerp(MinFlashGlowIntensity, ActiveGlowIntensity, SineVal);
				}
				break;
			case EStepVisualState::Success:
				TargetColor = SuccessColor;
				TargetIntensity = ActiveGlowIntensity;
				break;
			case EStepVisualState::Error:
				TargetColor = ErrorColor;
				TargetIntensity = ActiveGlowIntensity;
				break;
			case EStepVisualState::Off:
			default:
				TargetColor = InactiveColor;
				TargetIntensity = 0.0f;
				break;
		}

		// Interpolate/Apply changes only if they differ from cached values (except when flashing, which changes constantly)
		const float CurrentIntensity = PatternStepIntensities.FindRef(StepMesh);
		const FLinearColor CurrentColor = PatternStepColors.FindRef(StepMesh);

		if (VisualState == EStepVisualState::Flashing ||
			!FMath::IsNearlyEqual(CurrentIntensity, TargetIntensity, 0.001f) ||
			!CurrentColor.Equals(TargetColor, 0.001f))
		{
			(*MIDPtr)->SetVectorParameterValue(EmissiveColorParamName, TargetColor);
			(*MIDPtr)->SetScalarParameterValue(EmissiveParamName, TargetIntensity);

			PatternStepIntensities.Add(StepMesh, TargetIntensity);
			PatternStepColors.Add(StepMesh, TargetColor);
		}
	}
}

void AResonancePuzzleManager::HandleBellResonance(AActor* RungBell)
{
	if (bInErrorState || bPuzzleSolved)
	{
		return;
	}

	// Error if rung outside the resonance window
	const bool bIsResonanceWindow = MetronomeSteps.Num() > 0 && MetronomeSteps[CurrentMetronomeStepIndex].bIsResonanceWindow;
	if (!bIsResonanceWindow)
	{
		TriggerFailure();
		return;
	}

	// Error if we already completed a step this cycle
	if (bStepCompletedThisCycle)
	{
		TriggerFailure();
		return;
	}

	// Match against the expected step
	if (CurrentPatternStepIndex < ResolvedPuzzleSteps.Num())
	{
		const FResolvedPuzzleStep& CurrentStep = ResolvedPuzzleSteps[CurrentPatternStepIndex];

		// Check if the rung bell matches the expected actor
		if (!CurrentStep.bIsPauseStep && CurrentStep.ExpectedBellActor == RungBell)
		{
			// Correct bell resonated!
			bStepCompletedThisCycle = true;
			OnStepSucceeded(CurrentPatternStepIndex);
			return;
		}
	}

	// Any other case is a mistake (wrong bell or ringing during a pause step)
	TriggerFailure();
}

void AResonancePuzzleManager::OnStepSucceeded(const int32 StepIndex)
{
	if (StepIndex < 0 || StepIndex >= ResolvedPuzzleSteps.Num()) return;

	// Visual response: Keep both bells and pause dots lit when completed successfully
	SetPatternStepVisualState(StepIndex, EStepVisualState::Success);

	// Play step success sound using optimized audio component
	if (StepSuccessSound && FeedbackAudioComponent)
	{
		FeedbackAudioComponent->SetSound(StepSuccessSound);
		FeedbackAudioComponent->Play();
	}

	// Advance step
	CurrentPatternStepIndex++;

	if (CurrentPatternStepIndex >= ResolvedPuzzleSteps.Num())
	{
		TriggerSuccess();
	}
	else
	{
		// Set next step to start flashing
		SetPatternStepVisualState(CurrentPatternStepIndex, EStepVisualState::Flashing);
	}
}

void AResonancePuzzleManager::TriggerFailure(bool bIsTimeout)
{
	if (bInErrorState) return;

	// If it is a timeout (time ran out) and the player has not completed at least the first step,
	// do not trigger error feedback and let the metronome continue its cycle normally.
	if (bIsTimeout && CurrentPatternStepIndex == 0)
	{
		bStepCompletedThisCycle = false;
		return;
	}

	bInErrorState = true;

	// Broadcast failure event
	OnPuzzleFailed.Broadcast();

	// Play fail sound using optimized audio component
	if (PuzzleFailureSound && FeedbackAudioComponent)
	{
		FeedbackAudioComponent->SetSound(PuzzleFailureSound);
		FeedbackAudioComponent->Play();
	}

	// All symbols turn Error
	for (int32 i = 0; i < ResolvedPatternStepMeshes.Num(); ++i)
	{
		SetPatternStepVisualState(i, EStepVisualState::Error);
	}

	// Start error reset timer
	FTimerHandle ErrorTimerHandle;
	GetWorldTimerManager().SetTimer(ErrorTimerHandle, this, &AResonancePuzzleManager::ResetPuzzleAfterError, ErrorDuration, false);
}

void AResonancePuzzleManager::ResetPuzzleAfterError()
{
	bInErrorState = false;
	CurrentPatternStepIndex = 0;
	bStepCompletedThisCycle = false;

	// Reset metronome index and timer
	CurrentMetronomeStepIndex = 0;
	StepTimer = 0.0f;

	if (AreAllBellsInside())
	{
		// Turn all steps off, set first step back to flashing
		if (ResolvedPatternStepMeshes.Num() > 0)
		{
			SetPatternStepVisualState(0, EStepVisualState::Flashing);
			for (int32 i = 1; i < ResolvedPatternStepMeshes.Num(); ++i)
			{
				SetPatternStepVisualState(i, EStepVisualState::Off);
			}
		}

		// Play start step sound using optimized audio component
		if (MetronomeSteps.Num() > 0 && MetronomeSteps[0].StepSound && MetronomeAudioComponent)
		{
			MetronomeAudioComponent->SetSound(MetronomeSteps[0].StepSound);
			MetronomeAudioComponent->Play();
		}
	}
	else
	{
		InitializePuzzleDormant();
	}
}

void AResonancePuzzleManager::TriggerSuccess()
{
	if (bPuzzleSolved) return;

	bPuzzleSolved = true;

	// Broadcast solved event
	OnPuzzleSolved.Broadcast();

	// Play victory sound using optimized audio component
	if (PuzzleSuccessSound && FeedbackAudioComponent)
	{
		FeedbackAudioComponent->SetSound(PuzzleSuccessSound);
		FeedbackAudioComponent->Play();
	}

	// Trigger target interactable actors if configured
	for (AActor* TargetActor : SuccessTargetActors)
	{
		if (TargetActor)
		{
			if (IInteractable* Interactable = Cast<IInteractable>(TargetActor))
			{
				Interactable->TriggerInteraction(this);
			}
		}
	}
}

void AResonancePuzzleManager::SetPatternStepVisualState(const int32 StepIndex, const EStepVisualState NewState)
{
	if (StepIndex >= 0 && StepIndex < ResolvedPatternStepMeshes.Num())
	{
		UStaticMeshComponent* StepMesh = ResolvedPatternStepMeshes[StepIndex];
		if (StepMesh)
		{
			PatternStepVisualStates.FindOrAdd(StepMesh) = NewState;
		}
	}
}

void AResonancePuzzleManager::UpdateSuccessGlow(const float DeltaTime)
{
	const float TargetIntensity = bPuzzleSolved ? ActiveGlowIntensity : 0.0f;
	bool bGlowFinished = true;

	for (UStaticMeshComponent* GlowMesh : ResolvedSuccessGlowComponents)
	{
		if (!GlowMesh) continue;

		const TObjectPtr<UMaterialInstanceDynamic>* MIDPtr = SuccessGlowMIDs.Find(GlowMesh);
		if (!MIDPtr || !*MIDPtr) continue;

		float CurrentVal = SuccessGlowIntensities.FindRef(GlowMesh);
		const float NewVal = FMath::FInterpTo(CurrentVal, TargetIntensity, DeltaTime, GlowBlendSpeed);
		if (!FMath::IsNearlyEqual(CurrentVal, NewVal, 0.001f))
		{
			(*MIDPtr)->SetScalarParameterValue(EmissiveParamName, NewVal);
			SuccessGlowIntensities.Add(GlowMesh, NewVal);
			bGlowFinished = false;
		}
	}

	// Disable Tick entirely when the puzzle is solved and the success glow has finished transitioning
	if (bPuzzleSolved && bGlowFinished)
	{
		SetActorTickEnabled(false);
	}
}

void AResonancePuzzleManager::CheckInitialBellsOverlapping()
{
	BellsInsideArea.Empty();
	if (!ActionAreaCollider) return;

	TArray<AActor*> OverlappingActors;
	ActionAreaCollider->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && ResolvedBells.Contains(Actor))
		{
			BellsInsideArea.Add(Actor);
		}
	}
}

void AResonancePuzzleManager::CheckInitialBellsAndStart()
{
	CheckInitialBellsOverlapping();

	if (AreAllBellsInside())
	{
		StartPuzzle();
	}
	else
	{
		InitializePuzzleDormant();
	}
}

bool AResonancePuzzleManager::AreAllBellsInside() const
{
	return ResolvedBells.Num() > 0 && BellsInsideArea.Num() == ResolvedBells.Num();
}

void AResonancePuzzleManager::StartPuzzle()
{
	if (bPuzzleActive || bPuzzleSolved) return;

	bPuzzleActive = true;
	CurrentPatternStepIndex = 0;
	StepTimer = 0.0f;
	CurrentMetronomeStepIndex = 0;

	// Set first step to flashing, others off
	if (ResolvedPatternStepMeshes.Num() > 0)
	{
		SetPatternStepVisualState(0, EStepVisualState::Flashing);
		for (int32 i = 1; i < ResolvedPatternStepMeshes.Num(); ++i)
		{
			SetPatternStepVisualState(i, EStepVisualState::Off);
		}
	}

	// Play start step sound using optimized audio component
	if (MetronomeSteps.Num() > 0 && MetronomeSteps[0].StepSound && MetronomeAudioComponent)
	{
		MetronomeAudioComponent->SetSound(MetronomeSteps[0].StepSound);
		MetronomeAudioComponent->Play();
	}
}

void AResonancePuzzleManager::InitializePuzzleDormant()
{
	bPuzzleActive = false;
	CurrentPatternStepIndex = 0;
	StepTimer = 0.0f;
	CurrentMetronomeStepIndex = 0;

	// Turn all steps off
	for (int32 i = 0; i < ResolvedPatternStepMeshes.Num(); ++i)
	{
		SetPatternStepVisualState(i, EStepVisualState::Off);
	}
}

void AResonancePuzzleManager::OnActionAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || bPuzzleSolved || bPuzzleActive || bInErrorState) return;

	if (ResolvedBells.Contains(OtherActor))
	{
		const bool bAlreadyInside = BellsInsideArea.Contains(OtherActor);
		BellsInsideArea.Add(OtherActor);

		// Also check if other bells are already inside to catch any missed at startup (TSet avoids duplicates)
		if (ActionAreaCollider)
		{
			TArray<AActor*> OverlappingActors;
			ActionAreaCollider->GetOverlappingActors(OverlappingActors);
			for (AActor* Actor : OverlappingActors)
			{
				if (Actor && ResolvedBells.Contains(Actor))
				{
					BellsInsideArea.Add(Actor);
				}
			}
		}

		if (!bAlreadyInside)
		{
			if (ARailMoveObj* RailMove = Cast<ARailMoveObj>(OtherActor))
			{
				// RailMoveObj saves itself when it reaches the end of the rail
			}
			else
			{
				if (IRestorable* Restorable = Cast<IRestorable>(OtherActor))
				{
					Restorable->SaveCheckpointState();
				}

				if (const UWorld* World = GetWorld())
				{
					if (UGameInstance* GI = World->GetGameInstance())
					{
						if (USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
						{
							if (SaveSubsystem->GetCurrentSlotSave() > 0)
							{
								SaveSubsystem->RequestSaveSlotAsync(SaveSubsystem->GetCurrentSlotSave());
							}
						}
					}
				}
			}
		}

		if (AreAllBellsInside())
		{
			StartPuzzle();
		}
	}
}

void AResonancePuzzleManager::OnActionAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || bPuzzleSolved) return;

	if (ResolvedBells.Contains(OtherActor))
	{
		BellsInsideArea.Remove(OtherActor);

		if (bPuzzleActive)
		{
			InitializePuzzleDormant();
		}
	}
}
