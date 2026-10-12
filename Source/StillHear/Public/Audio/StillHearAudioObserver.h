#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "StillHearAudioObserver.generated.h"

class UAudioStateComponent;

/**
 * Universal World Subsystem that manages audio state requests from any source.
 */
UCLASS()
class STILLHEAR_API UStillHearAudioObserver : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Register or update a state request from a component */
	void RegisterStateRequest(UAudioStateComponent* Source, FGameplayTag Tag, bool bActive);

private:
	/** Map of active requests: Source Component -> Tag */
	UPROPERTY()
	TMap<TWeakObjectPtr<UAudioStateComponent>, FGameplayTag> ActiveRequests;

	/** Set of tags that we have currently pushed to the subsystem */
	UPROPERTY()
	TSet<FGameplayTag> CurrentActiveTags;

	/** Refresh the global audio state based on all active requests */
	void RefreshGlobalAudioState();
};
