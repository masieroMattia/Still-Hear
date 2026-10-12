#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "AudioStateConfig.generated.h"

/**
 * Struct representing the audio configuration for a specific game state.
 */
USTRUCT(BlueprintType)
struct FAudioStateInfo
{
	GENERATED_BODY()

	/** The Gameplay Tag associated with this state (e.g., State.Audio.Combat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	FGameplayTag StateTag;

	/** The priority of this state. Higher priority states override lower ones. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	int32 Priority = 0;

	/** If true, this state will attempt to change the music */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	bool bOverrideMusic = true;

	/** If true, this state will attempt to change the ambience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	bool bOverrideAmbience = true;

	/** Volume multiplier for music in this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	float MusicVolumeMultiplier = 1.0f;

	/** Volume multiplier for ambience in this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	float AmbienceVolumeMultiplier = 1.0f;

	/** If true, this state will not be cleared when changing levels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	bool bPersistAcrossLevels = false;

	/** If true, this sound will continue to play when the game is paused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	bool bIsUISound = true;

	/** The music track for this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	TSoftObjectPtr<USoundBase> MusicTrack;

	/** The ambience track for this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	TSoftObjectPtr<USoundBase> AmbienceTrack;

	/** How fast to fade in when entering this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	float FadeInTime = 1.0f;

	/** How fast to fade out the previous state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
	float FadeOutTime = 1.0f;
};

/**
 * Data Asset that maps Gameplay Tags to specific Audio States.
 */
UCLASS(BlueprintType)
class STILLHEAR_API UAudioStateConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** List of all audio states and their associated sounds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Configuration")
	TArray<FAudioStateInfo> States;

	/** Find the audio info for a specific tag */
	const FAudioStateInfo* FindStateInfo(const FGameplayTag& Tag) const
	{
		for (const FAudioStateInfo& Info : States)
		{
			if (Info.StateTag.MatchesTagExact(Tag))
			{
				return &Info;
			}
		}
		return nullptr;
	}
};
