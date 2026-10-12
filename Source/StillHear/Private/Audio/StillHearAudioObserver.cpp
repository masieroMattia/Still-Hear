#include "Audio/StillHearAudioObserver.h"
#include "Audio/GameAudioSubsystem.h"
#include "Audio/AudioStateComponent.h"

void UStillHearAudioObserver::RegisterStateRequest(UAudioStateComponent* Source, FGameplayTag Tag, bool bActive)
{
	if (!Source) return;

	if (bActive)
	{
		ActiveRequests.Add(Source, Tag);
	}
	else
	{
		ActiveRequests.Remove(Source);
	}

	RefreshGlobalAudioState();
}

void UStillHearAudioObserver::RefreshGlobalAudioState()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameAudioSubsystem* AudioSS = World->GetGameInstance()->GetSubsystem<UGameAudioSubsystem>();
	if (!AudioSS) return;

	// 1. Identify all unique tags requested currently
	TSet<FGameplayTag> NewRequestedTags;
	for (auto It = ActiveRequests.CreateIterator(); It; ++It)
	{
		if (It.Key().IsValid())
		{
			NewRequestedTags.Add(It.Value());
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	// 2. Identify tags that were active but are no longer requested (POP)
	TArray<FGameplayTag> TagsToPop;
	for (const FGameplayTag& Tag : CurrentActiveTags)
	{
		if (!NewRequestedTags.Contains(Tag))
		{
			TagsToPop.Add(Tag);
		}
	}

	for (const FGameplayTag& Tag : TagsToPop)
	{
		AudioSS->PopAudioState(Tag);
		CurrentActiveTags.Remove(Tag);
	}

	// 3. Identify new tags that are now requested (PUSH)
	for (const FGameplayTag& Tag : NewRequestedTags)
	{
		if (!CurrentActiveTags.Contains(Tag))
		{
			AudioSS->PushAudioState(Tag);
			CurrentActiveTags.Add(Tag);
		}
	}
}
