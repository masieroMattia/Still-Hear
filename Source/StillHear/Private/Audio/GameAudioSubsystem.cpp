#include "Audio/GameAudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/SettingsSaveGame.h"


void UGameAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    SoundSettings = GetDefault<USoundDeveloperSettings>();
    LoadSettingsAssets();

    // Bind to level loading to clean up the stack
    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UGameAudioSubsystem::OnWorldLoaded);
    FWorldDelegates::OnPostWorldCreation.AddUObject(this, &ThisClass::HandleWorldLoaded);
    
    if(GEngine)
    {
        for(const FWorldContext & context : GEngine->GetWorldContexts())
        {
            UWorld* world = context.World();
            if(world && world->IsGameWorld())
            {
                HandleWorldLoaded(world);
            }
        }
    }
    
    Levels = SoundSettings ? SoundSettings->DefaultLevels : FSoundLevels{};
}

void UGameAudioSubsystem::OnWorldLoaded(UWorld* World, const UWorld::InitializationValues IVS)
{
    if (!World || !World->IsGameWorld()) return;

    // IMPORTANT: Ensure this world belongs to this GameInstance (Multi-PIE safety)
    if (World->GetGameInstance() != GetGameInstance()) return;

    // Remove all states that shouldn't persist across levels
    if (AudioStateConfig)
    {
        AudioStateStack.RemoveAll([this](const FGameplayTag& Tag)
        {
            const FAudioStateInfo* Info = AudioStateConfig->FindStateInfo(Tag);
            const bool bRemove = Info ? !Info->bPersistAcrossLevels : true;
            return bRemove; 
        });
    }
    
    RefreshAudioState(true);
}

void UGameAudioSubsystem::Deinitialize()
{
    if (MusicComp)
    {
        MusicComp->Stop();
        MusicComp->DestroyComponent();
        MusicComp = nullptr;
    }

    if (AmbienceComp)
    {
        AmbienceComp->Stop();
        AmbienceComp->DestroyComponent();
        AmbienceComp = nullptr;
    }
    
    if (RuntimeMix)
    {
        if (UWorld* AudioWorld = GetAudioWorld())
        {
            UGameplayStatics::PopSoundMixModifier(AudioWorld, RuntimeMix);
        }
    }
    bRuntimeMixInitialized = false;
    
    /*
     * Clearing the pointers isn't strictly necessary here,
     * just a good habit.
     */
    SoundSettings = nullptr;
    CurrentWorld = nullptr;
    ActiveMusicStateTag = FGameplayTag::EmptyTag;
    ActiveAmbienceStateTag = FGameplayTag::EmptyTag;
    FWorldDelegates::OnPostWorldCreation.RemoveAll(this);
    Super::Deinitialize();
}

void UGameAudioSubsystem::EnsureMusicComponent(USoundBase* SoundToPlay)
{
    if (MusicComp && IsValid(MusicComp)) return;

    UWorld* World = GetAudioWorld();
    if (!World) return;

    MusicComp = UGameplayStatics::CreateSound2D(World, SoundToPlay, 1.0f, 1.0f, 0.0f, nullptr, true, false);
    if (MusicComp)
    {
        MusicComp->bAutoDestroy = false;
        MusicComp->bIsUISound = true;
        MusicComp->SoundClassOverride = SC_Music;
        // Keep it alive in the GameInstance
        MusicComp->SetFlags(RF_Standalone); 
    }
}

void UGameAudioSubsystem::EnsureAmbienceComponent(USoundBase* SoundToPlay)
{
    if (AmbienceComp && IsValid(AmbienceComp)) return;

    UWorld* World = GetAudioWorld();
    if (!World) return;

    AmbienceComp = UGameplayStatics::CreateSound2D(World, SoundToPlay, 1.0f, 1.0f, 0.0f, nullptr, true, false);
    if (AmbienceComp)
    {
        AmbienceComp->bAutoDestroy = false;
        AmbienceComp->bIsUISound = true;
        AmbienceComp->SoundClassOverride = SC_Ambience;
        AmbienceComp->SetFlags(RF_Standalone);
    }
}

void UGameAudioSubsystem::PlayMusic(USoundBase* Track, float FadeIn, float Volume)
{
    if (!Track) return;

    EnsureMusicComponent(Track);
    if (!MusicComp) return;

    CurrentMusic = Track;
    
    MusicComp->SetSound(Track);
    MusicComp->SetVolumeMultiplier(Volume);
    MusicComp->FadeIn(FadeIn, Volume);
}


void UGameAudioSubsystem::CrossfadeMusic(USoundBase* NewTrack, float FadeOutOld, float FadeInNew, float Volume)
{
    if (!NewTrack) return;
    
    UWorld* AudioWorld = GetAudioWorld();
    if (!AudioWorld) return;

    // IMPORTANT: Clear any pending crossfade to avoid race conditions
    AudioWorld->GetTimerManager().ClearTimer(MusicTimerHandle);

    EnsureMusicComponent(NewTrack);
    if (!MusicComp) return;

    if (MusicComp->IsPlaying())
    {
        MusicComp->FadeOut(FadeOutOld, 0.0f);
        
        TWeakObjectPtr<USoundBase> WeakTrack(NewTrack);
        AudioWorld->GetTimerManager().SetTimer(MusicTimerHandle, [this, WeakTrack, FadeInNew, Volume]()
        {
            USoundBase* Track = WeakTrack.Get();
            if (!Track) return;               
            if (!IsValid(MusicComp)) return;  
            PlayMusic(Track, FadeInNew, Volume);
        }, FadeOutOld, false);
    }
    else
    {
        PlayMusic(NewTrack, FadeInNew, Volume);
    }
}

void UGameAudioSubsystem::StopMusic(float FadeOut)
{
    if (UWorld* AudioWorld = GetAudioWorld())
    {
        AudioWorld->GetTimerManager().ClearTimer(MusicTimerHandle);
    }

    if (!MusicComp) return;
    if (MusicComp->IsPlaying())
        MusicComp->FadeOut(FadeOut, 0.0f);
}

void UGameAudioSubsystem::PlayAmbience(USoundBase* Track, float FadeIn, float Volume)
{
    if (!Track) return;

    EnsureAmbienceComponent(Track);
    if (!AmbienceComp) return;

    CurrentAmbience = Track;
    
    AmbienceComp->SetSound(Track);
    AmbienceComp->SetVolumeMultiplier(Volume);
    AmbienceComp->FadeIn(FadeIn, Volume);
}

void UGameAudioSubsystem::CrossfadeAmbience(USoundBase* NewTrack, float FadeOutOld, float FadeInNew, float Volume)
{
    if (!NewTrack) return;

    UWorld* AudioWorld = GetAudioWorld();
    if (!AudioWorld) return;

    // IMPORTANT: Clear any pending crossfade to avoid race conditions
    AudioWorld->GetTimerManager().ClearTimer(AmbienceTimerHandle);

    EnsureAmbienceComponent(NewTrack);
    if (!AmbienceComp) return;

    if (AmbienceComp->IsPlaying())
    {
        AmbienceComp->FadeOut(FadeOutOld, 0.0f);
        
        TWeakObjectPtr<USoundBase> WeakTrack(NewTrack);
        AudioWorld->GetTimerManager().SetTimer(AmbienceTimerHandle, [this, WeakTrack, FadeInNew, Volume]()
        {
            USoundBase* Track = WeakTrack.Get();
            if (!Track) return;               
            if (!IsValid(AmbienceComp)) return;  
            PlayAmbience(Track, FadeInNew, Volume);
        }, FadeOutOld, false);
    }
    else
    {
        PlayAmbience(NewTrack, FadeInNew, Volume);
    }
}

void UGameAudioSubsystem::StopAmbience(float FadeOut)
{
    if (UWorld* AudioWorld = GetAudioWorld())
    {
        AudioWorld->GetTimerManager().ClearTimer(AmbienceTimerHandle);
    }

    if (!AmbienceComp) return;
    if (AmbienceComp->IsPlaying())
        AmbienceComp->FadeOut(FadeOut, 0.0f);
}


void UGameAudioSubsystem::PushAudioState(FGameplayTag StateTag)
{
    if (!StateTag.IsValid()) return;

    // Security: Prevent stack bloating
    if (AudioStateStack.Num() > 100)
    {
        AudioStateStack.RemoveAt(0);
    }

    // Avoid duplicate pushes of the same tag if it's already in the stack
    if (AudioStateStack.Contains(StateTag))
    {
        return;
    }

    AudioStateStack.Add(StateTag);
    RefreshAudioState();
}

void UGameAudioSubsystem::PopAudioState(FGameplayTag StateTag)
{
    if (AudioStateStack.Num() == 0)
    {
        return;
    }

    // If a specific tag is provided, remove all instances of it
    if (StateTag.IsValid())
    {
        AudioStateStack.RemoveAll([StateTag](const FGameplayTag& T) { return T.MatchesTagExact(StateTag); });
    }
    else
    {
        // Otherwise just pop the last one
        AudioStateStack.Pop();
    }

    RefreshAudioState();
}

void UGameAudioSubsystem::ClearAudioStack(float FadeOut)
{
    AudioStateStack.Empty();
    StopMusic(FadeOut);
    StopAmbience(FadeOut);
    ActiveMusicStateTag = FGameplayTag::EmptyTag;
    ActiveAmbienceStateTag = FGameplayTag::EmptyTag;
}

FGameplayTag UGameAudioSubsystem::GetCurrentAudioState() const
{
    if (AudioStateStack.Num() == 0) return FGameplayTag::EmptyTag;

    if (!AudioStateConfig) return AudioStateStack.Last();

    // Find the tag with the highest priority in the current stack
    FGameplayTag BestTag = FGameplayTag::EmptyTag;
    int32 BestPriority = -1;

    for (const FGameplayTag& Tag : AudioStateStack)
    {
        if (const FAudioStateInfo* Info = AudioStateConfig->FindStateInfo(Tag))
        {
            if (Info->Priority >= BestPriority)
            {
                BestPriority = Info->Priority;
                BestTag = Tag;
            }
        }
    }

    return BestTag.IsValid() ? BestTag : AudioStateStack.Last();
}

void UGameAudioSubsystem::RefreshAudioState(bool bForce)
{
    if (!AudioStateConfig) return;

    // If stack is empty, stop all audio or return to silence
    if (AudioStateStack.Num() == 0)
    {
        float MusicFadeOut = 1.0f;
        float AmbienceFadeOut = 1.0f;

        // Retrieve the fade out time of the last active states before returning to silence
        if (ActiveMusicStateTag.IsValid())
        {
            if (const FAudioStateInfo* PrevInfo = AudioStateConfig->FindStateInfo(ActiveMusicStateTag))
            {
                MusicFadeOut = PrevInfo->FadeOutTime;
            }
        }

        if (ActiveAmbienceStateTag.IsValid())
        {
            if (const FAudioStateInfo* PrevInfo = AudioStateConfig->FindStateInfo(ActiveAmbienceStateTag))
            {
                AmbienceFadeOut = PrevInfo->FadeOutTime;
            }
        }

        StopMusic(MusicFadeOut);
        StopAmbience(AmbienceFadeOut);
        
        CurrentMusic = nullptr;
        CurrentAmbience = nullptr;
        ActiveMusicStateTag = FGameplayTag::EmptyTag;
        ActiveAmbienceStateTag = FGameplayTag::EmptyTag;
        return;
    }

    const FAudioStateInfo* BestMusicState = nullptr;
    const FAudioStateInfo* BestAmbienceState = nullptr;
    
    int32 MaxMusicPriority = -1;
    int32 MaxAmbiencePriority = -1;

    // 1. Find the best state for each channel independently based on priority and overrides
    for (const FGameplayTag& Tag : AudioStateStack)
    {
        const FAudioStateInfo* Info = AudioStateConfig->FindStateInfo(Tag);
        if (!Info) continue;

        if (Info->bOverrideMusic && Info->Priority >= MaxMusicPriority)
        {
            MaxMusicPriority = Info->Priority;
            BestMusicState = Info;
        }

        if (Info->bOverrideAmbience && Info->Priority >= MaxAmbiencePriority)
        {
            MaxAmbiencePriority = Info->Priority;
            BestAmbienceState = Info;
        }
    }

    // 2. Apply Music change if needed
    if (BestMusicState)
    {
        USoundBase* MusicAsset = BestMusicState->MusicTrack.LoadSynchronous();
        bool bMusicChanged = bForce || (CurrentMusic != MusicAsset);

        if (bMusicChanged)
        {
            EnsureMusicComponent(MusicAsset);
            float FinalMultiplier = BestMusicState->MusicVolumeMultiplier;
            
            if (MusicComp)
            {
                MusicComp->bIsUISound = BestMusicState->bIsUISound;
                MusicComp->SetVolumeMultiplier(FinalMultiplier);
            }

            // Retrieve the fade out time of the previously active music state, if any
            float FadeOutTime = BestMusicState->FadeOutTime;
            if (ActiveMusicStateTag.IsValid())
            {
                if (const FAudioStateInfo* PrevInfo = AudioStateConfig->FindStateInfo(ActiveMusicStateTag))
                {
                    FadeOutTime = PrevInfo->FadeOutTime;
                }
            }

            if (MusicAsset)
            {
                CrossfadeMusic(MusicAsset, FadeOutTime, BestMusicState->FadeInTime, FinalMultiplier);
            }
            else
            {
                StopMusic(FadeOutTime);
            }

            CurrentMusic = MusicAsset;
            ActiveMusicStateTag = BestMusicState->StateTag;
        }
        else if (MusicComp)
        {
            // Update volume multiplier even if track is same
            MusicComp->SetVolumeMultiplier(BestMusicState->MusicVolumeMultiplier);
            ActiveMusicStateTag = BestMusicState->StateTag;
        }
    }

    // 3. Apply Ambience change if needed
    if (BestAmbienceState)
    {
        USoundBase* AmbienceAsset = BestAmbienceState->AmbienceTrack.LoadSynchronous();
        bool bAmbienceChanged = bForce || (CurrentAmbience != AmbienceAsset);

        if (bAmbienceChanged)
        {
            EnsureAmbienceComponent(AmbienceAsset);
            float FinalMultiplier = BestAmbienceState->AmbienceVolumeMultiplier;

            // Retrieve the fade out time of the previously active ambience state, if any
            float FadeOutTime = BestAmbienceState->FadeOutTime;
            if (ActiveAmbienceStateTag.IsValid())
            {
                if (const FAudioStateInfo* PrevInfo = AudioStateConfig->FindStateInfo(ActiveAmbienceStateTag))
                {
                    FadeOutTime = PrevInfo->FadeOutTime;
                }
            }

            if (AmbienceAsset)
            {
                CrossfadeAmbience(AmbienceAsset, FadeOutTime, BestAmbienceState->FadeInTime, FinalMultiplier);
            }
            else
            {
                StopAmbience(FadeOutTime);
            }

            CurrentAmbience = AmbienceAsset;
            ActiveAmbienceStateTag = BestAmbienceState->StateTag;
        }
        else if (AmbienceComp)
        {
            // Update volume even if asset is same
            AmbienceComp->SetVolumeMultiplier(BestAmbienceState->AmbienceVolumeMultiplier);
            ActiveAmbienceStateTag = BestAmbienceState->StateTag;
        }
    }
}

void UGameAudioSubsystem::SetAudioStateStack(const TArray<FGameplayTag>& NewStack)
{
    AudioStateStack = NewStack;
    RefreshAudioState(true);
}

void UGameAudioSubsystem::SetMasterVolume(float V) {Levels.Master = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_Master, Levels.Master, 0.05f); }
void UGameAudioSubsystem::SetMusicVolume(float V) {Levels.Music = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_Music,  Levels.Music, 0.05f);}
void UGameAudioSubsystem::SetAmbienceVolume(float V) {Levels.Ambience = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_Ambience, Levels.Ambience, 0.05f); }
void UGameAudioSubsystem::SetSFXVolume(float V)    {Levels.SFX = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_SFX,    Levels.SFX, 0.05f); }
void UGameAudioSubsystem::SetUIVolume(float V)     {Levels.UI = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_UI,     Levels.UI, 0.05f); }
void UGameAudioSubsystem::SetVoiceVolume(float V)  {Levels.Voice = FMath::Clamp(V,0.f,1.f); ApplyClassVolumeRuntime(SC_Voice,  Levels.Voice, 0.05f); }

void UGameAudioSubsystem::SetLevels(const FSoundLevels& newLevels)
{
    SetMasterVolume(newLevels.Master);
    SetMusicVolume(newLevels.Music);
    SetAmbienceVolume(newLevels.Ambience);
    SetSFXVolume(newLevels.SFX);
    SetUIVolume(newLevels.UI);
    SetVoiceVolume(newLevels.Voice);
}


void UGameAudioSubsystem::InitRuntimeMix()
{
    if (bRuntimeMixInitialized) return;
    if (!RuntimeMix) return;
    
    UWorld* AudioWorld = GetAudioWorld();
    if (!AudioWorld) return;

    UGameplayStatics::SetBaseSoundMix(AudioWorld, RuntimeMix);
    UGameplayStatics::PushSoundMixModifier(AudioWorld, RuntimeMix);

    bRuntimeMixInitialized = true;
}


void UGameAudioSubsystem::ApplyClassVolumeRuntime(USoundClass* SC, float Volume01, float FadeTime)
{
    if (!SC || !RuntimeMix) return;
    
    UWorld* AudioWorld = GetAudioWorld();
    if (!AudioWorld) return;
    
    InitRuntimeMix();
    
    Volume01 = FMath::Clamp(Volume01, 0.0f, 1.0f);
    UGameplayStatics::SetSoundMixClassOverride(
        AudioWorld,
        RuntimeMix,
        SC,
        Volume01,
        1.0f,
        FadeTime,
        true
    );
    // Broadcast the change
    OnLevelChanged.Broadcast(SC, Volume01);
}

void UGameAudioSubsystem::SetWorldSFXDynamics(float Volume01, float FadeTime)
{
    Volume01 = FMath::Clamp(Volume01, 0.f, 1.f);
    ApplyClassVolumeRuntime(SC_SFX_Gameplay, Volume01, FadeTime);
}

void UGameAudioSubsystem::SetMusicDucking(float Multiplier, float FadeTime)
{
    const float Target = Levels.Music * FMath::Clamp(Multiplier, 0.f, 1.f);
    ApplyClassVolumeRuntime(SC_Music, Target, FadeTime);
}

UWorld* UGameAudioSubsystem::GetAudioWorld() const
{
    return CurrentWorld.IsValid() ? const_cast<UWorld*>(CurrentWorld.Get()) : GetWorld();
}

void UGameAudioSubsystem::HandleWorldLoaded(UWorld* LoadedWorld)
{
    if (!LoadedWorld || !LoadedWorld->IsGameWorld()) return;

    if (CurrentWorld.IsValid() && CurrentWorld.Get() == LoadedWorld)
    {
        return;
    }

    CurrentWorld = LoadedWorld;
    bRuntimeMixInitialized = false;

    // Reset active state tags on world transition to rebuild state correctly
    ActiveMusicStateTag = FGameplayTag::EmptyTag;
    ActiveAmbienceStateTag = FGameplayTag::EmptyTag;

    // IMPORTANT: When loading a new world/save, we should only clear NON-PERSISTENT states.
    // Persistent states (like those from FlowGraph saved in SaveGame) should stay.
    TArray<FGameplayTag> StatesToKeep;
    if (AudioStateConfig)
    {
        for (const FGameplayTag& Tag : AudioStateStack)
        {
            const FAudioStateInfo* Info = AudioStateConfig->FindStateInfo(Tag);
            const bool bPersist = Info && Info->bPersistAcrossLevels;
            if (bPersist)
            {
                StatesToKeep.Add(Tag);
            }
        }
    }
    
    AudioStateStack = StatesToKeep;

    LoadedWorld->GetTimerManager().SetTimerForNextTick([this]()
    {
        InitRuntimeMix();
        SetLevels(Levels);
        SetVolumeFromSaveSettings();
        RefreshAudioState(true);
    });
}

void UGameAudioSubsystem::LoadSettingsAssets()
{
    if (!SoundSettings) return;
    
    

    RuntimeMix  = SoundSettings->SMX_RunTimeSettings.IsNull() ? nullptr : SoundSettings->SMX_RunTimeSettings.LoadSynchronous();

    SC_Master   = SoundSettings->SC_Master.IsNull()   ? nullptr : SoundSettings->SC_Master.LoadSynchronous();
    SC_Music    = SoundSettings->SC_Music.IsNull()    ? nullptr : SoundSettings->SC_Music.LoadSynchronous();
    SC_Ambience = SoundSettings->SC_Ambience.IsNull() ? nullptr : SoundSettings->SC_Ambience.LoadSynchronous();
    SC_SFX      = SoundSettings->SC_SFX.IsNull()      ? nullptr : SoundSettings->SC_SFX.LoadSynchronous();
    SC_UI       = SoundSettings->SC_UI.IsNull()       ? nullptr : SoundSettings->SC_UI.LoadSynchronous();
    SC_Voice    = SoundSettings->SC_Voice.IsNull()    ? nullptr : SoundSettings->SC_Voice.LoadSynchronous();
    SC_SFX_Gameplay = SoundSettings->SC_SFX_Gameplay.IsNull() ? nullptr : SoundSettings->SC_SFX_Gameplay.LoadSynchronous();
    
    // Load state config from settings
    if (!AudioStateConfig && SoundSettings)
    {
        AudioStateConfig = SoundSettings->AudioStateConfig.IsNull() ? nullptr : SoundSettings->AudioStateConfig.LoadSynchronous();
    }

    PauseDuckMultiplier = SoundSettings->PauseDuckMultiplier;
    DuckFadeTime = SoundSettings->DuckFadeTime;
}

void UGameAudioSubsystem::SetVolumeFromSaveSettings()
{
   
    UGameInstance* GI = GetGameInstance();
    if (!GI && GetWorld()) GI = GetWorld()->GetGameInstance();
    if (GI)
    {
        if (USaveSubsystem* SaveSS = GI->GetSubsystem<USaveSubsystem>())
        {
            // SettingsSaveGame = SaveSS->GetSaveSettings(); 
            
            if (USettingsSaveGame* S = SaveSS->GetSettings())
            {
                SetMasterVolume(S->MasterVolume);
                SetMusicVolume(S->MusicVolume);
                SetSFXVolume(S->SfxVolume);
                SetVoiceVolume(S->VoiceVolume);
                SetAmbienceVolume(S->AmbienceVolume);
            }
        }
    }
    
}





