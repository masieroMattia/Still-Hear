#include "VFX/ResonanceManagerComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Data/DataAssets/ResonanceData.h"
#include "Camera/CameraEffects/CameraEffectsComponent.h"

// Define Static Parameter Names
const FName FResonanceParamNames::HeightTop = FName("User.HeightTop");
const FName FResonanceParamNames::HeightBottom = FName("User.HeightBottom");
const FName FResonanceParamNames::TriggerShockwave = FName("User.TriggerShockwave");

#pragma region CONSTRUCTOR
UResonanceManagerComponent::UResonanceManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
#pragma endregion
	
#pragma region METHODS
void UResonanceManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ResonanceData)
		CurrentHeight = ResonanceData->MaxHeight;
	
	SetComponentTickEnabled(false);
}

void UResonanceManagerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ResonanceData || !ResonanceData->ResonanceCurve) 
		return;

	UpdateLogic(DeltaTime);
	UpdateVFXParameters();
	UpdateSoundState();
}

void UResonanceManagerComponent::StartResonance()
{	
	if (Phase1VFX && Phase1VFX->IsActive())
		Phase1VFX->DeactivateImmediate();

	if (Phase2VFX && Phase2VFX->IsActive())
		Phase2VFX->DeactivateImmediate();
	
	CurrentPhase = EResonancePhase::Inactive;
	PhaseElapsedTime = 0.0f;
	bInThreshold = false;
	
	if (ResonanceData)
		CurrentHeight = ResonanceData->MaxHeight;

	USoundBase* EntrySound = ResonanceData->EntryResonanceSound;
	if (EntrySound)
		UGameplayStatics::PlaySoundAtLocation(this, EntrySound, GetOwner()->GetActorLocation());
	
	HandlePhaseTransition(EResonancePhase::Phase1);
}

void UResonanceManagerComponent::AttemptMatch()
{
	if (!ResonanceData) 
		return;
    
	// Prevent restarting if the resonance is already inactive or successful
	if (CurrentPhase == EResonancePhase::Inactive || CurrentPhase == EResonancePhase::Success) 
		return;
    
	const float AbsHeight = FMath::Abs(CurrentHeight);
	const float HalfThreshold = ResonanceData->MatchThreshold * 0.5f;

	if (AbsHeight <= HalfThreshold)
	{
		if (CurrentPhase == EResonancePhase::Phase1)
			HandlePhaseTransition(EResonancePhase::Phase2); // Success Phase 1->Smooth Reset then Phase 2
		else if (CurrentPhase == EResonancePhase::Phase2)
			HandlePhaseTransition(EResonancePhase::Success); // Success Phase 2->Finish
	}
	else
	{
		if (CurrentPhase == EResonancePhase::Phase2)
		{
			HandleInterruption();
		}
		else
		{
			HandlePhaseTransition(EResonancePhase::Reset); // Phase1 fail -> smooth reset back to Phase1
		}
	}
}

void UResonanceManagerComponent::StopResonance()
{
	if (SpawnedSound)
	{
		SpawnedSound->Stop();
		SpawnedSound = nullptr;
	}
	
	HandlePhaseTransition(EResonancePhase::Inactive);
}

void UResonanceManagerComponent::HandlePhaseTransition(const EResonancePhase NewPhase)
{
	if (!ResonanceData) 
		return;

    // Transitioning to Phase 2 or Reset
    if ((NewPhase == EResonancePhase::Phase2 || NewPhase == EResonancePhase::Reset) && CurrentPhase != EResonancePhase::Reset)
    {
        if (NewPhase == EResonancePhase::Phase2)
        {
        	bPhase2Unlocked = true;
            Phase2VFX = GetOrCreateVFX(ResonanceData->Phase2VFX, Phase2VFX);
            if (Phase2VFX)
            {
            	Phase2VFX->SetVisibility(true);
				Phase2VFX->Activate(true);
            }
        }
        else
        {
        	USoundBase* FailSound = ResonanceData->FailResonanceSound;
        	if (FailSound)
        		UGameplayStatics::PlaySoundAtLocation(this, FailSound, GetOwner()->GetActorLocation());
        }
    	
    	CurrentHeight = FMath::Abs(CurrentHeight);
        CurrentPhase = EResonancePhase::Reset;
        return;
    }

    CurrentPhase = NewPhase;
    PhaseElapsedTime = 0.0f;

    // Cache curve duration
	if (ResonanceData->ResonanceCurve)
	{
		// GetTimeRange gives the start and end time of the curve
		float MinTime, MaxTime;
		ResonanceData->ResonanceCurve->GetTimeRange(MinTime, MaxTime);
		CurrentPhaseDuration = MaxTime;
	}

    switch (CurrentPhase)
    {
        case EResonancePhase::Phase1:
	    {
    		bSuccessful = false;
    		bFailure = false;
    		bInThreshold = false;
    	
    		if (Phase2VFX)
    			Phase2VFX->SetVariableBool(FResonanceParamNames::TriggerShockwave, false);
    	
            SetComponentTickEnabled(true);
    	
    		if (Phase2VFX && Phase2VFX->IsActive())
    			Phase2VFX->Deactivate();
    	
    		Phase1VFX = GetOrCreateVFX(ResonanceData->Phase1VFX, Phase1VFX);
    	
            if (Phase1VFX)
            {
            	Phase1VFX->SetVisibility(true);
            	if (!Phase1VFX->IsActive())
            		Phase1VFX->Activate(true);
            }
    	
    		if (!SpawnedSound || !SpawnedSound->IsPlaying())
    		{
    			USoundBase* ActiveSound = ResonanceData->ActiveResonanceSound;
    			if (ActiveSound)
    				SpawnedSound = UGameplayStatics::SpawnSoundAttached(ActiveSound, GetOwner()->GetRootComponent());
    		}
            break;
	    }

		case EResonancePhase::Phase2:
		{
    		bPhase2Unlocked = false;
    		bInThreshold = false;
    	
    		USoundBase* Phase2Sound = ResonanceData->Phase2ResonanceSound;
    		if (Phase2Sound)
    			UGameplayStatics::PlaySoundAtLocation(this, Phase2Sound, GetOwner()->GetActorLocation());

        	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        	const APlayerController* OwnerPlayerController = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
			if (OwnerPlayerController)
			{
				UCameraEffectsComponent* CameraEffectComponent = OwnerPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass()) ? Cast<UCameraEffectsComponent>(OwnerPlayerController->GetComponentByClass(UCameraEffectsComponent::StaticClass())) : nullptr;
				if (CameraEffectComponent)
					CameraEffectComponent->PlayEffectPreset(ResonanceData->Phase2EffectPreset);
			}
        		
		    SetComponentTickEnabled(true);
		    break;
		}
    	
        case EResonancePhase::Success:
	    {
    		bSuccessful = true;
    		if (Phase1VFX && Phase1VFX->IsActive())
    			Phase1VFX->Deactivate();
    		if (Phase2VFX && Phase2VFX->IsActive())
    		{
    			Phase2VFX->SetVariableBool(FResonanceParamNames::TriggerShockwave, true);
    			Phase2VFX->SetAutoDestroy(true);
    			Phase2VFX->Deactivate();
    			Phase2VFX = nullptr;
    		}
    	
    		if (SpawnedSound)
			{
				SpawnedSound->Stop();
				SpawnedSound = nullptr;
			}
    	
    		USoundBase* SuccessSound = ResonanceData->SuccessResonanceSound;
    		if (SuccessSound)
    			UGameplayStatics::PlaySoundAtLocation(this, SuccessSound, GetOwner()->GetActorLocation());
    	
            OnResonanceSuccess.Broadcast();
    		SetComponentTickEnabled(false);
            break;
	    }

        case EResonancePhase::Inactive:
        {
    		if (Phase1VFX && Phase1VFX->IsActive()) 
    			Phase1VFX->Deactivate();
    		if (Phase2VFX && !bSuccessful && Phase2VFX->IsActive())
    			Phase2VFX->Deactivate();
    		SetComponentTickEnabled(false);
			break;
        }
        default: 
    		break;
    }
}

void UResonanceManagerComponent::HandleInterruption()
{
	if (!ResonanceData)
		return;

	if (SpawnedSound)
	{
		SpawnedSound->Stop();
		SpawnedSound = nullptr;
	}

	if (Phase1VFX && Phase1VFX->IsActive())
		Phase1VFX->Deactivate();
	if (Phase2VFX && Phase2VFX->IsActive())
		Phase2VFX->Deactivate();

	if (ResonanceData->InterruptResonanceSound)
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceData->InterruptResonanceSound, GetOwner()->GetActorLocation());

	if (ResonanceData->InterruptVFX)
		UNiagaraFunctionLibrary::SpawnSystemAttached(ResonanceData->InterruptVFX, GetOwner()->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

	SetComponentTickEnabled(false);
	CurrentPhase = EResonancePhase::Inactive;

	OnResonanceInterrupted.Broadcast();
}

void UResonanceManagerComponent::UpdateLogic(const float DeltaTime)
{
	if (CurrentPhase == EResonancePhase::Reset)
	{
		CurrentHeight = FMath::FInterpTo(CurrentHeight, ResonanceData->MaxHeight, DeltaTime, ResonanceData->ResetSpeed);
        
		if (FMath::IsNearlyEqual(CurrentHeight, ResonanceData->MaxHeight, 1.0f))
		{
			// Snap to exact value to prevent slight offsets in the next phase
			CurrentHeight = ResonanceData->MaxHeight;
			
			if (!bFailure && bPhase2Unlocked)
				HandlePhaseTransition(EResonancePhase::Phase2);
			else
				HandlePhaseTransition(EResonancePhase::Phase1);
		}
		return; 
	}
	
	float FinalPlayRate = 1.0f;
	if (CurrentPhase == EResonancePhase::Phase2)
		FinalPlayRate = ResonanceData->SpeedMultiplier;
   
	PhaseElapsedTime += DeltaTime * FinalPlayRate;
	
	if (CurrentPhase == EResonancePhase::Phase1)
	{
		// Use Fmod to keep exact timing preventing drift over multiple loops
		if (PhaseElapsedTime > CurrentPhaseDuration)
			PhaseElapsedTime = FMath::Fmod(PhaseElapsedTime, CurrentPhaseDuration);
	}
	else if (CurrentPhase == EResonancePhase::Phase2)
	{
		// One-shot failure for Phase 2: interrupt instead of resetting to Phase1
		if (PhaseElapsedTime > CurrentPhaseDuration)
		{
			HandleInterruption();
			return;
		}
	}
	
	const float CurveValue = ResonanceData->ResonanceCurve->GetFloatValue(PhaseElapsedTime);
	CurrentHeight = CurveValue * ResonanceData->MaxHeight;
}

void UResonanceManagerComponent::UpdateVFXParameters() const
{
	if (Phase1VFX && Phase1VFX->IsActive())
	{
		Phase1VFX->SetVariableFloat(FResonanceParamNames::HeightTop, CurrentHeight);
		Phase1VFX->SetVariableFloat(FResonanceParamNames::HeightBottom, -CurrentHeight);
	}
}

void UResonanceManagerComponent::UpdateSoundState()
{
	if (!ResonanceData) 
		return;

	const float AbsHeight = FMath::Abs(CurrentHeight);
	const float HalfThreshold = ResonanceData->MatchThreshold * 0.5f;
	const bool  bInZone = (AbsHeight <= HalfThreshold);

	if (bInZone && !bInThreshold)
	{
		USoundBase* CollideSound = ResonanceData->CollideResonanceSound;
		if (CollideSound)
			UGameplayStatics::PlaySoundAtLocation(this, CollideSound, GetOwner()->GetActorLocation());

	}

	bInThreshold = bInZone;
}

float UResonanceManagerComponent::GetCurrentCirclesDistance() const
{
	return FMath::Abs(CurrentHeight * 2.0f);
}

EResonancePhase UResonanceManagerComponent::GetCurrentPhase() const
{
	return CurrentPhase;
}

UNiagaraComponent* UResonanceManagerComponent::GetOrCreateVFX(const TObjectPtr<UNiagaraSystem> System, TObjectPtr<UNiagaraComponent>& Field)
{
	if (Field) 
		return Field;
	if (System)
	{
		Field = UNiagaraFunctionLibrary::SpawnSystemAttached(System, GetOwner()->GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		if (Field)
			Field->SetVisibility(true);
	}
	return Field;
}
#pragma endregion
