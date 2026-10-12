

#pragma once

#include "CoreMinimal.h"
#include "SoundDeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Audio/AudioGameConfig.h"
#include "Audio/AudioStateConfig.h"
#include "Audio/SoundDelegates.h"
#include "GameplayTagContainer.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "GameAudioSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UGameAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// Fields
public:	
	UPROPERTY(BlueprintAssignable, Category = "Audio Events")
	FSoundLevelDelegate OnLevelChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio Config")
	UAudioGameConfig* Config = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config")
	TObjectPtr<UAudioStateConfig> AudioStateConfig;
	
private:
	UPROPERTY()
	TWeakObjectPtr<const UWorld> CurrentWorld;
	
	UPROPERTY()
	TObjectPtr<const class USoundDeveloperSettings> SoundSettings;
	
	UPROPERTY()
	FSoundLevels Levels;
	
	bool bRuntimeMixInitialized = false;
	
	// Persistent
	UPROPERTY(Transient) UAudioComponent* MusicComp = nullptr;
	UPROPERTY(Transient) UAudioComponent* AmbienceComp = nullptr;

	UPROPERTY(Transient) USoundBase* CurrentMusic = nullptr;
	UPROPERTY(Transient) USoundBase* CurrentAmbience = nullptr;
	
	UPROPERTY(Transient)
	TArray<FGameplayTag> AudioStateStack;

	UPROPERTY(Transient)
	FGameplayTag ActiveMusicStateTag;

	UPROPERTY(Transient)
	FGameplayTag ActiveAmbienceStateTag;


	UPROPERTY(EditAnywhere, Category = "Audio|Config")
	FGameplayTag CombatStateTag;

	// Fraction of the normal music volume used while the game is paused (from Project Settings > Game Sound)
	float PauseDuckMultiplier = 0.3f;
	// Time it takes to fade the music volume in/out when ducking/unducking (from Project Settings > Game Sound)
	float DuckFadeTime = 0.3f;
	
	UPROPERTY(Transient) TObjectPtr<USoundMix> RuntimeMix = nullptr;

	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_Master = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_Music = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_Ambience = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_SFX = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_UI = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_Voice = nullptr;
	UPROPERTY(Transient) TObjectPtr<USoundClass> SC_SFX_Gameplay = nullptr;
	
	FTimerHandle MusicTimerHandle;
	FTimerHandle AmbienceTimerHandle;
	
	// Methods
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	FORCEINLINE const class USoundDeveloperSettings* GetSettings() const { return SoundSettings; }
	const FSoundLevels& GetCurrentLevels() const { return Levels; }

	// ---- Music controls ----
	UFUNCTION(BlueprintCallable, Category="Audio Music")
	void PlayMusic(USoundBase* Track, float FadeIn = 0.5f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Audio Music")
	void CrossfadeMusic(USoundBase* NewTrack, float FadeOutOld = 0.5f, float FadeInNew = 0.5f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Audio Music")
	void StopMusic(float FadeOut = 0.5f);

	// ---- Ambience controls ----
	UFUNCTION(BlueprintCallable, Category="Audio Ambience")
	void PlayAmbience(USoundBase* Track, float FadeIn = 0.5f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Audio Ambience")
	void CrossfadeAmbience(USoundBase* NewTrack, float FadeOutOld = 0.5f, float FadeInNew = 0.5f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category="Audio Ambience")
	void StopAmbience(float FadeOut = 0.5f);

	// ---- Volume settings ----
	
	// Master Volume
	UFUNCTION(BlueprintPure, Category = "Audio Settings")
	FORCEINLINE float GetMasterVolume() const { return Levels.Master; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetMasterVolume(float Volume01);
	
    // Music Volume
	UFUNCTION(BlueprintPure, Category="Audio Settings")
	FORCEINLINE float GetMusicVolume() const { return Levels.Music; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetMusicVolume(float Volume01);
	
	// Ambience Volume
	UFUNCTION(BlueprintPure, Category="Audio Settings")
	FORCEINLINE float GetAmbienceVolume() const { return Levels.Ambience; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetAmbienceVolume(float Volume01);
   	
	// SFX Volume
	UFUNCTION(BlueprintPure, Category="Audio Settings")
	FORCEINLINE float GetSFXVolume() const { return Levels.SFX; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetSFXVolume(float Volume01);
    
	// UI Volume
	UFUNCTION(BlueprintPure, Category="Audio Settings")
	FORCEINLINE float GetUIVolume() const { return Levels.UI; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetUIVolume(float Volume01);
	
	// Voice Volume
	UFUNCTION(BlueprintPure, Category="Audio Settings")
	FORCEINLINE float GetVoiceVolume() const { return Levels.Voice; }
	UFUNCTION(BlueprintCallable, Category="Audio Settings")
	void SetVoiceVolume(float Volume01);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Dynamics")
	void SetWorldSFXDynamics(float Volume01, float FadeTime = 0.05f);

	/** Scales the music volume by Multiplier (1.0 = normal), fading over FadeTime seconds */
	UFUNCTION(BlueprintCallable, Category="Audio|Dynamics")
	void SetMusicDucking(float Multiplier, float FadeTime);

	// Configured fraction of the normal music volume to use while the game is paused
	UFUNCTION(BlueprintPure, Category="Audio|Dynamics")
	FORCEINLINE float GetPauseDuckMultiplier() const { return PauseDuckMultiplier; }

	// Configured fade time for ducking/unducking the music volume
	UFUNCTION(BlueprintPure, Category="Audio|Dynamics")
	FORCEINLINE float GetDuckFadeTime() const { return DuckFadeTime; }

	// ---- State-Driven Audio ----
	/** 
	 * Adds a state to the stack and evaluates the new top state.
	 * If the new state has higher or equal priority than current, it will crossfade.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|States")
	void PushAudioState(FGameplayTag StateTag);

	/** 
	 * Removes a state from the stack and returns to the previous one.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|States")
	void PopAudioState(FGameplayTag StateTag);

	/** Clears all states from the stack and stops all audio */
	UFUNCTION(BlueprintCallable, Category = "Audio|States")
	void ClearAudioStack(float FadeOut = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Audio|States")
	FGameplayTag GetCurrentAudioState() const;

	FGameplayTag GetCombatStateTag() const { return CombatStateTag; }

	/** Internal helper to refresh the audio based on the current stack */
	void RefreshAudioState(bool bForce = false);

	void OnWorldLoaded(UWorld* World, const UWorld::InitializationValues IVS);

	// Persistency helpers
	const TArray<FGameplayTag>& GetAudioStateStack() const { return AudioStateStack; }
	void SetAudioStateStack(const TArray<FGameplayTag>& NewStack);

private:
	UFUNCTION(BlueprintCallable)
	void ApplyClassVolumeRuntime(USoundClass* SC, float Volume01, float FadeTime = 0.5f);

	/*
	 * Follows a set of function to interact with submixes
	 * in a bulk.
	 */
	const FSoundLevels & GetLevels() const { return Levels; }
	UFUNCTION(BlueprintPure, Category = "Audio", DisplayName = "Get Levels")
	FSoundLevels GetLevelsCopy() const { return Levels; }
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetLevels(const FSoundLevels & newLevels);

	UFUNCTION(BlueprintCallable)
	void SetVolumeFromSaveSettings();
private:
	
	
	UFUNCTION()
	void HandleWorldLoaded(UWorld* newWorld);
	
	UWorld* GetAudioWorld() const;
	void LoadSettingsAssets();
	
	
	
	
	
	void EnsureMusicComponent(USoundBase* SoundToPlay = nullptr);
	void EnsureAmbienceComponent(USoundBase* SoundToPlay = nullptr);
	void InitRuntimeMix();
	
};
