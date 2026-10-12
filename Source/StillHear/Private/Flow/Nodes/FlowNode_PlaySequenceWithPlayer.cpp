#include "Flow/Nodes/FlowNode_PlaySequenceWithPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "LevelSequenceActor.h"
#include "LevelSequence/FlowLevelSequencePlayer.h"

UFlowNode_PlaySequenceWithPlayer::UFlowNode_PlaySequenceWithPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("Cinematic");
#endif
}

void UFlowNode_PlaySequenceWithPlayer::ExecuteInput(const FName& PinName)
{
	if (PinName == TEXT("Start"))
	{
		LoadedSequence = Sequence.LoadSynchronous();

		if (GetWorld() && LoadedSequence)
		{
			// Inline creation logic to capture SequenceActor directly
			ALevelSequenceActor* SequenceActor = nullptr;

			AActor* OwningActor = TryGetRootFlowActorOwner();

			// Apply AActor::CustomTimeDilation from owner of the Root Flow
			if (IsValid(OwningActor))
			{
				PlaybackSettings.PlayRate = CachedPlayRate * OwningActor->CustomTimeDilation;
			}

			// Apply Transform Origin
			AActor* TransformOriginActor = bUseGraphOwnerAsTransformOrigin ? OwningActor : nullptr;

			// Create the player and capture the spawned SequenceActor reference
			SequencePlayer = UFlowLevelSequencePlayer::CreateFlowLevelSequencePlayer(
				this, 
				LoadedSequence, 
				PlaybackSettings, 
				CameraSettings, 
				TransformOriginActor, 
				bReplicates, 
				bAlwaysRelevant, 
				SequenceActor
			);

			if (SequencePlayer)
			{
				SequencePlayer->SetFlowEventReceiver(this);
			}

			const FFrameRate FrameRate = LoadedSequence->GetMovieScene()->GetTickResolution();
			const FFrameNumber PlaybackStartFrame = LoadedSequence->GetMovieScene()->GetPlaybackRange().GetLowerBoundValue();
			StartTime = FQualifiedFrameTime(FFrameTime(PlaybackStartFrame, 0.0f), FrameRate).AsSeconds();

			if (SequencePlayer)
			{
				// Perform the binding using the captured SequenceActor
				if (SequenceActor)
				{
					ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
					if (PlayerCharacter)
					{
						// Bind the player character using the tag "Player"
						SequenceActor->SetBindingByTag(FName("Player"), TArray<AActor*>{ PlayerCharacter });
					}
				}

				TriggerOutput(TEXT("PreStart"));

				OnSkipRequested.AddUObject(this, &UFlowNode_PlayLevelSequence::SkipPlayback);
				SequencePlayer->OnFinished.AddDynamic(this, &UFlowNode_PlaySequenceWithPlayer::OnPlaybackFinished);

				if (bPlayReverse)
				{
					SequencePlayer->PlayReverse();
				}
				else
				{
					SequencePlayer->Play();
				}

				TriggerOutput(TEXT("Started"));

				OnPlaybackStarted.Broadcast();
			}
		}

		TriggerFirstOutput(false);
	}
	else
	{
		Super::ExecuteInput(PinName);
	}
}
