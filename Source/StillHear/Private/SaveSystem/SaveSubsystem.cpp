#include "SaveSystem/SaveSubsystem.h"

#include "DLSSLibrary.h"
#include "FlowSubsystem.h"
#include "Flow/SceneManager.h"
#include "SaveSystem/Savable.h"
#include "HAL/IConsoleManager.h"
#include "SaveSystem/SaveTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Audio/GameAudioSubsystem.h"
#include "SaveSystem/SaveIdComponent.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "SaveSystem/FSaveCoreArchive.h"
#include "Audio/SoundDeveloperSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Data/DataTables/CheckpointDisplayData.h"
#include "Internationalization/Internationalization.h"
#include "DevTools/DevToolsSettings.h"

class UGameAudioSubsystem;

// INIT
void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &USaveSubsystem::OnLevelAddedToWorld);
	FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &USaveSubsystem::OnLevelRemovedFromWorld);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USaveSubsystem::OnPostLoadMap);
	LoadSettings();
	ApplyUpscalerFromSettings();
	ApplyLanguageFromSettings();
	
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
		{
			FlowSubsystem->OnCheckpointRequestedSave.AddDynamic(this, &USaveSubsystem::HandleFlowCheckpointSaveRequested);
		}
	}
}

//DEINIT
void USaveSubsystem::Deinitialize()
{
	FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

// Helpers to get stable keys for levels/maps
FName USaveSubsystem::GetStableMapKey(const UWorld* World)
{
	if (!World) 
		return NAME_None;

	const UPackage* Pkg = World->GetPackage();
	if (!Pkg) 
		return NAME_None;

	FString Name = Pkg->GetName(); 
	
#if WITH_EDITOR
	Name = UWorld::RemovePIEPrefix(Name);
#endif
	
	return FName(*Name);
}

// Serialization helpers
void USaveSubsystem::SerializeObject(UObject* Object, TArray<uint8>& OutBytes)
{
	OutBytes.Reset();
	if (!Object) 
		return;

	FMemoryWriter MemWriter(OutBytes, true);
	FSaveCoreArchive Ar(MemWriter);
	Object->Serialize(Ar);
}

void USaveSubsystem::DeserializeObject(UObject* Object, const TArray<uint8>& InBytes)
{
	if (!Object || InBytes.Num() == 0) 
		return;

	FMemoryReader MemReader(InBytes, true);
	FSaveCoreArchive Ar(MemReader);
	Object->Serialize(Ar);
}

FName USaveSubsystem::GetStableLevelKey(const UWorld* World, const ULevel* Level)
{
	if (!World || !Level) 
		return NAME_None;
	
	// Fallback/standard: use level package name
	const UPackage* Pkg = Level->GetPackage();
	if (!Pkg) 
		return NAME_None;

	FString LevelPkgName = Pkg->GetName();
#if WITH_EDITOR
	LevelPkgName = UWorld::RemovePIEPrefix(LevelPkgName);
#endif

	// Loop over streaming levels to see if we can match
	for (const ULevelStreaming* LS : World->GetStreamingLevels())
	{
		if (!LS) continue;
		
		// Method 1: Direct pointer match
		if (LS->GetLoadedLevel() == Level)
		{
			const FName AssetPkg = LS->GetWorldAssetPackageFName();
			if (AssetPkg != NAME_None)
			{
				FString Name = AssetPkg.ToString();
#if WITH_EDITOR
				Name = UWorld::RemovePIEPrefix(Name);
#endif
				return FName(*Name);
			}
		}

		// Method 2: Match by package name (robust fallback when loaded level pointer is not set yet)
		const FName AssetPkg = LS->GetWorldAssetPackageFName();
		if (AssetPkg != NAME_None)
		{
			FString AssetPkgStr = AssetPkg.ToString();
#if WITH_EDITOR
			AssetPkgStr = UWorld::RemovePIEPrefix(AssetPkgStr);
#endif

			// If the level package name is exactly equal to the asset package,
			// or starts with the asset package followed by an instance suffix (e.g. "_Instance_", "_LevelInstance_", "_")
			if (LevelPkgName.Equals(AssetPkgStr, ESearchCase::IgnoreCase) || LevelPkgName.StartsWith(AssetPkgStr + TEXT("_"), ESearchCase::IgnoreCase))
			{
				return FName(*AssetPkgStr);
			}
		}
	}

	return FName(*LevelPkgName);
}

// Post-load map hook
// ─── Upscaler ──────────────────────────────────────────────────────────────
void USaveSubsystem::ApplyUpscalerFromSettings()
{
	if (!Settings) return;

	const FString& Upscaler = Settings->ActiveUpscaler;

	// Helper to set r.ScreenPercentage preserving existing CVar priority
	auto SetScreenPct = [](float Value)
	{
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
		{
			const EConsoleVariableFlags P = static_cast<EConsoleVariableFlags>(CV->GetFlags() & ECVF_SetByMask);
			CV->Set(Value, P != 0 ? P : ECVF_SetByGameSetting);
		}
	};

	auto DisableFSR = []()
	{
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.Enabled")))
			CV->Set(0, ECVF_SetByGameSetting);
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FI.Enabled")))
			CV->Set(0, ECVF_SetByGameSetting);
	};

	auto DisableDLSS = [&SetScreenPct]()
	{
		UDLSSLibrary::EnableDLSS(false);
		SetScreenPct(100.0f);
	};

	if (Upscaler == TEXT("DLSS"))
	{
		DisableFSR();

		// Map saved index → DLSS mode
		const int32 SRIdx = Settings->DLSS_SuperResolutionIndex;
		UDLSSMode TargetMode;
		switch (SRIdx)
		{
			case 0:  DisableDLSS(); return; // OFF
			case 1:  TargetMode = UDLSSMode::Auto;             break;
			case 2:  TargetMode = UDLSSMode::DLAA;             break;
			case 3:  TargetMode = UDLSSMode::UltraQuality;     break;
			case 4:  TargetMode = UDLSSMode::Quality;          break;
			case 5:  TargetMode = UDLSSMode::Balanced;         break;
			case 6:  TargetMode = UDLSSMode::Performance;      break;
			case 7:  TargetMode = UDLSSMode::UltraPerformance; break;
			default: TargetMode = UDLSSMode::Quality;          break;
		}
		if (TargetMode == UDLSSMode::Auto)
		{
			TargetMode = UDLSSLibrary::GetDefaultDLSSMode();
			if (TargetMode == UDLSSMode::Off || TargetMode == UDLSSMode::Auto)
				TargetMode = UDLSSMode::Quality;
		}
		if (TargetMode == UDLSSMode::DLAA)
		{
			SetScreenPct(100.0f);
			UDLSSLibrary::EnableDLSS(true);
		}
		else
		{
			FVector2D Resolution(1920.f, 1080.f);
			if (GEngine && GEngine->GameUserSettings)
			{
				const FIntPoint R = GEngine->GameUserSettings->GetScreenResolution();
				Resolution = FVector2D(R.X, R.Y);
			}
			bool bSupported; float OptSP; bool bFixed; float MinSP, MaxSP, Sharp;
			UDLSSLibrary::GetDLSSModeInformation(TargetMode, Resolution, bSupported, OptSP, bFixed, MinSP, MaxSP, Sharp);
			SetScreenPct(bSupported && OptSP > 0.f ? OptSP : 66.7f);
			UDLSSLibrary::EnableDLSS(true);
		}
	}
	else if (Upscaler == TEXT("FSR"))
	{
		DisableDLSS();

		const int32 SRIdx = Settings->FSR_SuperResolutionIndex;
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.Enabled")))
			CV->Set(SRIdx == 0 ? 0 : 1, ECVF_SetByGameSetting);
		if (SRIdx > 0)
		{
			if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.QualityMode")))
				CV->Set(SRIdx, ECVF_SetByGameSetting);
		}
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FI.Enabled")))
			CV->Set(Settings->FSR_FrameGenerationIndex, ECVF_SetByGameSetting);
	}
	else // TSR or unknown
	{
		DisableFSR();
		DisableDLSS();
		if (IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod")))
			CV->Set(4, ECVF_SetByGameSetting); // 4 = TSR
	}
}

void USaveSubsystem::ApplyLanguageFromSettings()
{
	if (!Settings || Settings->Language.IsEmpty())
		return;

	FInternationalization::Get().SetCurrentCulture(Settings->Language);
}

void USaveSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
	// Always re-apply upscaler settings after a map transition (independent of save slot)
	ApplyUpscalerFromSettings();

	if (!CurrentSave) return;
	
	const TArray<ULevel*>& Levels = LoadedWorld->GetLevels();
	for (ULevel* Level : Levels)
	{
		if (Level)
			ApplyLevel(Level, false);
	}
}

void USaveSubsystem::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (!InLevel || !InWorld) return ;
	if (InWorld != GetWorld()) return ;

	if (!CurrentSave)
	{
		CurrentSave = Cast<USaveGameObject>(
			UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass())
		);
	}
	
	ApplyLevel(InLevel, true);
}

// Level removed hook (streaming)
void USaveSubsystem::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (!InLevel || !InWorld) return;

	if (!CurrentSave) return;
	if (InWorld->bIsTearingDown) return;
	if (InLevel == InWorld->PersistentLevel) return;
	if (InWorld != GetWorld()) return;
	
	if (const ASceneManager* SM = GetSceneManager())
	{
		if (SM->IsInMenuMode())
			return;
	}

	SaveLevelInto(CurrentSave, InLevel);
}
// Save/Load functions

// Synchronous save (not used for now)
bool USaveSubsystem::SaveToSlot(int32 const SlotIndex, int32 const UserIndex)
{
	if (bSaveInProgress) return false;
	if (SlotIndex < 1) return false;

	OnSaveStarted.Broadcast();

	const FString SlotName = MakeSlotName(SlotIndex);

	PendingSlotName = SlotName;
	PendingUserIndex = UserIndex;

	USaveGameObject* SaveObj = Cast<USaveGameObject>(
		UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass())
	);
	if (!SaveObj)
	{
		OnSaveFinished.Broadcast(false);
		return false;
	}

	BuildSaveObject(SaveObj);

	CurrentSave = SaveObj;

	const bool bOk = UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
	OnSaveFinished.Broadcast(bOk);
	return bOk;
}

// Asynchronous save 
bool USaveSubsystem::RequestSaveSlotAsync(int32 SlotIndex)
{
	if (bSaveInProgress) return false; // or queue?

	if (SlotIndex < 1) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	bSaveInProgress = true;
	OnSaveStarted.Broadcast();

	PendingSlotName  = MakeSlotName(SlotIndex);
	PendingUserIndex = 0;
	
	if (CurrentSave)
	{
		PendingSaveObject = DuplicateObject(CurrentSave,this);
	}
	else
	{
	// Make snapshot to save if no current save
	PendingSaveObject = Cast<USaveGameObject>(
	  UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass())
	);
		
	}
	
	if (!PendingSaveObject)
	{
		bSaveInProgress = false;
		OnSaveFinished.Broadcast(false);
		return false;
	}
	// Save all loaded levels (this works on game thread)
	BuildSaveObject(PendingSaveObject);

	CurrentSave = PendingSaveObject;

	// Delegate for async completion
	FAsyncSaveGameToSlotDelegate SavedDelegate;
	SavedDelegate.BindUObject(this, &USaveSubsystem::HandleAsyncSaveComplete);

	// Async: serialize on game thread, write su worker thread, callback su game thread
	UGameplayStatics::AsyncSaveGameToSlot(PendingSaveObject, PendingSlotName, PendingUserIndex, SavedDelegate);

	return true;
}

bool USaveSubsystem::LoadFromSlot(int32 const SlotIndex, int32 const UserIndex, bool bRestoreAudio)
{
	if (bSaveInProgress && PendingSlotName == MakeSlotName(SlotIndex)) 
		return false;
	
	if (SlotIndex < 1) 
		return false;

	// Reset the previous session before loading the new one
	ResetGameSession();
	
	const FString SlotName = MakeSlotName(SlotIndex);

	CurrentSlot = SlotName;
	CurrentUserIndex = UserIndex;
	CurrentSlotIndex = SlotIndex;
	
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) 
	{
		return false;
	}
	
	CurrentSave = Cast<USaveGameObject>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!CurrentSave)
		return false;

	// Track when this slot was last opened so GetLatestSaveSlotIndex returns the most recently played slot
	CurrentSave->LastPlayedDate = FDateTime::UtcNow();
	UGameplayStatics::AsyncSaveGameToSlot(CurrentSave, SlotName, UserIndex, FAsyncSaveGameToSlotDelegate());

	ApplyLoadedDataToCurrentlyLoadedLevels();
	
	// FlowGraph: notify subsystem
	if (CurrentSave)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
			{
				TObjectPtr<UFlowSaveGame> FlowSave = NewObject<UFlowSaveGame>(this);
				FlowSave->FlowComponents = CurrentSave->FlowComponents;
				FlowSave->FlowInstances  = CurrentSave->FlowInstances;
				FlowSubsystem->OnGameLoaded(FlowSave);
			}

			// Audio State Persistence: Restore the last saved audio state stack
			if (UGameAudioSubsystem* AudioSS = GI->GetSubsystem<UGameAudioSubsystem>())
			{
				if (bRestoreAudio && CurrentSave->AudioStateStack.Num() > 0)
				{
					AudioSS->SetAudioStateStack(CurrentSave->AudioStateStack);
				}
			}
		}
	}
	
	return true;
}

void USaveSubsystem::ApplyLoadedDataToCurrentlyLoadedLevels()
{
	if (!CurrentSave) return ;
	UWorld* World = GetWorld();
	if (!World) return ;

	// Defer only the session's first apply — it can run mid-BeginPlay and race actors that capture their reset baseline there
	const bool bDeferApply = !bHasAppliedInitialLoad;
	bHasAppliedInitialLoad = true;

	for (const TArray<ULevel*>& Levels = World->GetLevels(); ULevel* Level : Levels)
	{
		if (Level)
			ApplyLevel(Level, bDeferApply);
	}
}

int USaveSubsystem::GetCurrentSlotSave() const
{
   	return CurrentSlotIndex;
}

int32 USaveSubsystem::GetLatestSaveSlotIndex() const
{
	int32 LatestSlot = -1;
	FDateTime LatestDate = FDateTime::MinValue();

	for (int32 i = 1; i <= MaxSaveSlots; ++i)
	{
		const FString SlotName = MakeSlotName(i);
		if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
			continue;

		if (const USaveGameObject* LoadedSave = Cast<USaveGameObject>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			const FDateTime DateToCompare = LoadedSave->LastPlayedDate > FDateTime::MinValue() ? LoadedSave->LastPlayedDate : LoadedSave->SaveDate;

			if (DateToCompare > LatestDate)
			{
				LatestDate = DateToCompare;
				LatestSlot = i;
			}
		}
	}

	return LatestSlot;
}

bool USaveSubsystem::HasAnySaveGames() const
{
	for (int32 i = 1; i <= MaxSaveSlots; ++i)
	{
		if (UGameplayStatics::DoesSaveGameExist(MakeSlotName(i), 0))
			return true;
	}
	return false;
}

bool USaveSubsystem::StartNewGame(int32 SlotIndex)
{
	if (SlotIndex == -1)
	{
		for (int32 i = 1; i <= MaxSaveSlots; ++i)
		{
			if (!UGameplayStatics::DoesSaveGameExist(MakeSlotName(i), 0))
			{
				SlotIndex = i;
				break;
			}
		}
	}

	if (SlotIndex < 1 || SlotIndex > MaxSaveSlots)
		return false;

	if (UWorld* World = GetWorld())
	{
		AActor* SMActor = UGameplayStatics::GetActorOfClass(World, ASceneManager::StaticClass());
		if (ASceneManager* SM = Cast<ASceneManager>(SMActor))
			return SM->LoadSlotAndTransition(SlotIndex);
	}

	ResetGameSession();

	USaveGameObject* NewSave = Cast<USaveGameObject>(
		UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass())
	);
	if (!NewSave)
		return false;

	NewSave->SaveVersion = 1;
	NewSave->SlotName = MakeSlotName(SlotIndex);
	NewSave->SaveDate = FDateTime::UtcNow();
	NewSave->LastLevelName = GetStableMapKey(GetWorld());

	const bool bSaved = UGameplayStatics::SaveGameToSlot(NewSave, NewSave->SlotName, 0);
	if (!bSaved)
		return false;

	return LoadFromSlot(SlotIndex, 0);
}

bool USaveSubsystem::LoadLatestSaveGame()
{
	const int32 LatestSlot = GetLatestSaveSlotIndex();
	if (LatestSlot <= 0)
		return false;

	if (UWorld* World = GetWorld())
	{
		AActor* SMActor = UGameplayStatics::GetActorOfClass(World, ASceneManager::StaticClass());
		if (ASceneManager* SM = Cast<ASceneManager>(SMActor))
			return SM->LoadSlotAndTransition(LatestSlot);
	}

	return LoadFromSlot(LatestSlot, 0);
}

bool USaveSubsystem::TryGetActorGuid(const AActor* Actor, FGuid& OutGuid)
{
	if (!Actor) return false;

	// Prefer GUID component
	if (const USaveIdComponent* IdComp = Actor->FindComponentByClass<USaveIdComponent>())
	{
		if (const FGuid Guid = IdComp->GetSaveId(); Guid.IsValid())
		{
			OutGuid = Guid;
			return true;
		}
	}
	
	return false;
}

void USaveSubsystem::HandleFlowCheckpointSaveRequested()
{
	RequestSaveSlotAsync(GetCurrentSlotSave());
}

void USaveSubsystem::ApplyLevel(ULevel* Level, bool bDeferToNextTick)
{
	if (!CurrentSave || !Level) 
		return;

	if (bDeferToNextTick)
	{
		UWorld* World = Level->GetWorld();
		if (!World) 
			return;

		TWeakObjectPtr<ULevel> WeakLevel = Level;
		World->GetTimerManager().SetTimerForNextTick([this, WeakLevel]()
		{
			if (ULevel* SafeLevel = WeakLevel.Get())
			{
				ApplyLevelInternal(SafeLevel);
			}
		});
	}
	else
	{
		ApplyLevelInternal(Level);
	}
}

void USaveSubsystem::ApplyLevelInternal(ULevel* Level)
{
	if (!Level || !CurrentSave) 
		return;

	UWorld* SafeWorld = Level->GetWorld();
	if (!SafeWorld)
		return;

	const FName LevelKey = GetStableLevelKey(SafeWorld, Level);
	const FLevelSaveData* LevelData = CurrentSave->Levels.Find(LevelKey);
	
	if (!LevelData) 
		return;

	TArray<AActor*> ActorsToDestroy;

	for (AActor* Actor : Level->Actors)
	{
		if (!IsValid(Actor)) 
			continue;

		FGuid Guid;
		if (!TryGetActorGuid(Actor, Guid)) 
			continue;

		const FActorSaveData* Saved = LevelData->Actors.Find(Guid);
		if (!Saved) 
		{
			ActorsToDestroy.Add(Actor);
			continue;
		}

		DeserializeObject(Actor, Saved->Bytes);
		Actor->SetActorTransform(Saved->Transform);

		// Deserialize components
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component && Component->GetClass()->ImplementsInterface(USavable::StaticClass()))
			{
				if (const FComponentSaveData* ComponentData = Saved->ComponentsBytes.Find(Component->GetFName()))
				{
					DeserializeObject(Component, ComponentData->Bytes);
					ISavable::Execute_OnPostLoad(Component);
				}
			}
		}

		if (Actor->GetClass()->ImplementsInterface(USavable::StaticClass()))
		{
			ISavable::Execute_OnPostLoad(Actor);
		}
	}

	for (AActor* Actor : ActorsToDestroy)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
}

void USaveSubsystem::BuildSaveObject(USaveGameObject* SaveObj)
{
	if (!SaveObj || !GetWorld()) return;
	
	SaveObj->SaveVersion = 1;
	SaveObj->SlotName = PendingSlotName;

	SaveObj->SaveDate = FDateTime::UtcNow();
	SaveObj->LastLevelName = GetStableMapKey(GetWorld());
	
	for (ULevel* Level : GetWorld()->GetLevels())
	{
		if (!Level) continue;
		SaveLevelInto(SaveObj, Level);
	}
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
		{
			TObjectPtr<UFlowSaveGame> FlowSave = NewObject<UFlowSaveGame>();

			FlowSubsystem->OnGameSaved(FlowSave);
			SaveObj->FlowComponents = FlowSave->FlowComponents;
			SaveObj->FlowInstances  = FlowSave->FlowInstances;
			
		}

		// Audio State Persistence: Store the current active audio state stack
		if (UGameAudioSubsystem* AudioSS = GI->GetSubsystem<UGameAudioSubsystem>())
		{
			SaveObj->AudioStateStack = AudioSS->GetAudioStateStack();
		}
	}
}


void USaveSubsystem::HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	bSaveInProgress = false;

	// Release reference
	PendingSaveObject = nullptr;

	OnSaveFinished.Broadcast(bSuccess);
}

void USaveSubsystem::SaveLevelInto(USaveGameObject* TargetSave, ULevel* Level)
{
	if (!TargetSave || !Level) return;

	UWorld* World = Level->GetWorld();
	if (!World) return;

	const FName LevelKey = GetStableLevelKey(World, Level);

	FLevelSaveData& LevelData = TargetSave->Levels.FindOrAdd(LevelKey);

	LevelData.Actors.Empty();

	for (AActor* Actor : Level->Actors)
	{
		if (!IsValid(Actor)) continue;
		SaveActorInto(Actor, LevelData);
	}
}

void USaveSubsystem::SaveActorInto(AActor* Actor, FLevelSaveData& LevelData)
{
	FGuid Guid;
	if (!TryGetActorGuid(Actor, Guid)) return;

	if (Actor->GetClass()->ImplementsInterface(USavable::StaticClass()))
	{
		ISavable::Execute_OnPreSave(Actor);
	}

	FActorSaveData Data;
	Data.Transform = Actor->GetActorTransform();

	SerializeObject(Actor, Data.Bytes);

	// Serialize components
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (Component && Component->GetClass()->ImplementsInterface(USavable::StaticClass()))
		{
			ISavable::Execute_OnPreSave(Component);
			FComponentSaveData ComponentData;
			SerializeObject(Component, ComponentData.Bytes);
			Data.ComponentsBytes.Add(Component->GetFName(), MoveTemp(ComponentData));
		}
	}

	LevelData.Actors.Add(Guid, MoveTemp(Data));
}

// Settings save/load
bool USaveSubsystem::LoadSettings()
{
	const FString Slot = SettingsSlotName();
	constexpr int32 UserIndex = 0;

	if (UGameplayStatics::DoesSaveGameExist(Slot, UserIndex))
	{
		Settings = Cast<USettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, UserIndex));
	}

	if (!Settings)
	{
		Settings = Cast<USettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));
	}

	return Settings != nullptr;
}

bool USaveSubsystem::SaveSettingsAsync()
{
	if (bSettingsSaveInProgress) 
		return false;
	
	if (!Settings) 
		return false;

	bSettingsSaveInProgress = true;
	OnSettingsSaveStarted.Broadcast();
	
	if (Settings)
	{
	PendingSettingsSave = Settings;
	}
	else
	{
		PendingSettingsSave = Cast<USettingsSaveGame>(
			UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass())
		);
	}
	
	if (!PendingSettingsSave)
	{
		bSettingsSaveInProgress = false;
		OnSettingsSaveFinished.Broadcast(false);
		return false;
	}

	const UGameInstance* GI = GetGameInstance();
	if (!GI) 
		return false;
	
	if (const UGameAudioSubsystem* AudioSS = GI->GetSubsystem<UGameAudioSubsystem>())
	{
		const FSoundLevels& L = AudioSS->GetCurrentLevels();
	
		PendingSettingsSave->MasterVolume = L.Master;
		PendingSettingsSave->MusicVolume  = L.Music;
		PendingSettingsSave->SfxVolume    = L.SFX;
		PendingSettingsSave->VoiceVolume  = L.Voice;
		PendingSettingsSave->AmbienceVolume = L.Ambience;
	}
	FAsyncSaveGameToSlotDelegate Delegate;
	Delegate.BindUObject(this, &USaveSubsystem::HandleAsyncSettingsSaveComplete);

	UGameplayStatics::AsyncSaveGameToSlot(PendingSettingsSave, SettingsSlotName(), 0, Delegate);
	return true;
}


void USaveSubsystem::HandleAsyncSettingsSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	bSettingsSaveInProgress = false;
	PendingSettingsSave = nullptr;
	OnSettingsSaveFinished.Broadcast(bSuccess);
}

// Debug Helper 
FString USaveSubsystem::DebugPrintCurrentSave() const
{
	if (!CurrentSave) return TEXT("No Current Save");

	FString Result;
	Result += FString::Printf(TEXT("Current Save Slot: %s\n"), *CurrentSlot);
	Result += FString::Printf(TEXT("Save Version: %d\n"), CurrentSave->SaveVersion);
	Result += FString::Printf(TEXT("Levels Saved: %d\n"), CurrentSave->Levels.Num());

	for (const auto& LevelPair : CurrentSave->Levels)
	{
		const FName& LevelName = LevelPair.Key;
		const FLevelSaveData& LevelData = LevelPair.Value;

		Result += FString::Printf(TEXT(" Level: %s, Actors: %d\n"), *LevelName.ToString(), LevelData.Actors.Num());

		for (const auto& ActorPair : LevelData.Actors)
		{
			const FGuid& ActorGuid = ActorPair.Key;
			const FActorSaveData& ActorData = ActorPair.Value;

			Result += FString::Printf(TEXT("  Actor GUID: %s, Bytes: %d\n"), *ActorGuid.ToString(), ActorData.Bytes.Num());
		}
		
		// Print also Flow Save Data
		if (CurrentSave->FlowComponents.Num() > 0 || CurrentSave->FlowInstances.Num() > 0)
		{
			Result += TEXT(" Flow Save Data Present\n");
			Result += FString::Printf(TEXT("  Flow Components: %d\n"), CurrentSave->FlowComponents.Num());
			Result += FString::Printf(TEXT("  Flow Instances: %d\n"), CurrentSave->FlowInstances.Num());
		}
		else
		{
			Result += TEXT(" No Flow Save Data\n");
		}
	}

	return Result;
}

void USaveSubsystem::SetCurrentSlotToNewGame(const int32 SlotIndex)
{
	ResetGameSession();

	CurrentSlot = MakeSlotName(SlotIndex);
	CurrentUserIndex = 0;
	CurrentSave = Cast<USaveGameObject>(
		UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass())
	);
	CurrentSlotIndex = SlotIndex;
}

bool USaveSubsystem::SkipToChapterFromBytes(const TArray<uint8>& SaveBytes)
{
	if (SaveBytes.Num() == 0)
		return false;

	const UDevToolsSettings* DevSettings = UDevToolsSettings::Get();
	const int32 ScratchSlot = DevSettings ? DevSettings->ScratchSlotIndex : 999;

	// Reconstruct the template save and materialize it on the scratch slot so the existing
	// full-reload pipeline (which checks DoesSaveGameExist) can pick it up unchanged.
	USaveGameObject* Tmpl = Cast<USaveGameObject>(UGameplayStatics::LoadGameFromMemory(SaveBytes));
	if (!Tmpl)
		return false;

	const FString SlotName = MakeSlotName(ScratchSlot);
	Tmpl->SlotName = SlotName;

	if (!UGameplayStatics::SaveGameToSlot(Tmpl, SlotName, 0))
		return false;

	if (UWorld* World = GetWorld())
	{
		if (ASceneManager* SM = Cast<ASceneManager>(
			UGameplayStatics::GetActorOfClass(World, ASceneManager::StaticClass())))
		{
			return SM->LoadSlotAndTransition(ScratchSlot, /*bAllowFullReload=*/true);
		}
	}

	return false;
}

#if WITH_EDITOR
bool USaveSubsystem::CaptureCurrentSaveToBytes(TArray<uint8>& OutBytes) const
{
	if (!CurrentSave)
		return false;

	return UGameplayStatics::SaveGameToMemory(CurrentSave, OutBytes);
}
#endif

void USaveSubsystem::ResetGameSession()
{
	CurrentSave = nullptr;
	CurrentSlotIndex = -1;
	CurrentSlot = TEXT("");

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
		{
			FlowSubsystem->AbortActiveFlows();
			FlowSubsystem->OnGameLoaded(nullptr);
		}

		if (UGameAudioSubsystem* AudioSS = GI->GetSubsystem<UGameAudioSubsystem>())
		{
			AudioSS->ClearAudioStack(0.0f);
		}
	}
}

void USaveSubsystem::SetCurrentCheckpoint(FName CheckpointId)
{
	if (CurrentSave)
		CurrentSave->CheckpointId = CheckpointId;
}

ASceneManager* USaveSubsystem::GetSceneManager() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	AActor* Actor = UGameplayStatics::GetActorOfClass(World, ASceneManager::StaticClass());
	return Cast<ASceneManager>(Actor);
}

bool USaveSubsystem::GetCheckpointDisplayData(FName CheckpointId, FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const
{
	const ASceneManager* SM = GetSceneManager();
	UDataTable* Table = SM ? SM->GetCheckpointDataTable() : nullptr;
	if (!Table)
		return false;

	// Use default row when no checkpoint has been reached yet
	const FName IdToUse = CheckpointId.IsNone() ? (SM ? SM->GetDefaultCheckpointId() : NAME_None) : CheckpointId;
	if (IdToUse.IsNone())
		return false;

	if (const FCheckpointDisplayData* Row = Table->FindRow<FCheckpointDisplayData>(IdToUse, TEXT("")))
	{
		OutName  = Row->DisplayName;
		OutImage = Row->BackgroundImage;
		return true;
	}
	return false;
}

bool USaveSubsystem::GetCurrentSaveCheckpointDisplayData(FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const
{
	const FName Id = CurrentSave ? CurrentSave->CheckpointId : NAME_None;
	return GetCheckpointDisplayData(Id, OutName, OutImage);
}

bool USaveSubsystem::GetSlotCheckpointDisplayData(int32 SlotIndex, FText& OutName, TSoftObjectPtr<UTexture2D>& OutImage) const
{
	if (SlotIndex < 1)
		return false;

	const FString SlotName = MakeSlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		return false;

	if (const USaveGameObject* Save = Cast<USaveGameObject>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		return GetCheckpointDisplayData(Save->CheckpointId, OutName, OutImage);

	return false;
}

void USaveSubsystem::CollectCollectible(FName RowName)
{
	if (!Settings || RowName.IsNone()) return;
	if (Settings->CollectedCollectibles.Contains(RowName)) return;
	Settings->CollectedCollectibles.Add(RowName);
	SaveSettingsAsync();
}

bool USaveSubsystem::IsCollectibleCollected(FName RowName) const
{
	return Settings && Settings->CollectedCollectibles.Contains(RowName);
}

TSet<FName> USaveSubsystem::GetCollectedCollectibles() const
{
	return Settings ? Settings->CollectedCollectibles : TSet<FName>();
}

void USaveSubsystem::ResetCollectibles()
{
	if (!Settings) return;
	Settings->CollectedCollectibles.Empty();
	SaveSettingsAsync();
}

bool USaveSubsystem::DeleteSlot(int32 const SlotIndex, int32 const UserIndex) const
{
	if (bSaveInProgress) 
		return false;

	const FString TargetSlotName = FString::Printf(TEXT("Save_Slot%d"), SlotIndex);

	// If it exists, delete it and return true if successful
	if (UGameplayStatics::DoesSaveGameExist(TargetSlotName, UserIndex))
	{
		const bool bDeleted = UGameplayStatics::DeleteGameInSlot(TargetSlotName, UserIndex);
		if (bDeleted)
		{
			if (UWorld* World = GetWorld())
			{
				AActor* SMActor = UGameplayStatics::GetActorOfClass(World, ASceneManager::StaticClass());
				if (ASceneManager* SM = Cast<ASceneManager>(SMActor))
					SM->RefreshMenuBackground();
			}
		}
		return bDeleted;
	}

	return false;
}

void USaveSubsystem::UnlockPermanentAbility(EMainCharacterAbilityType AbilityType)
{
	if (!CurrentSave)
	{
		CurrentSave = Cast<USaveGameObject>(UGameplayStatics::CreateSaveGameObject(USaveGameObject::StaticClass()));
	}

	if (CurrentSave)
	{
		if (!CurrentSave->UnlockedAbilities.Contains(AbilityType))
		{
			CurrentSave->UnlockedAbilities.Add(AbilityType);
		}
	}
}

void USaveSubsystem::RemovePermanentAbility(EMainCharacterAbilityType AbilityType)
{
	if (CurrentSave)
	{
		if (CurrentSave->UnlockedAbilities.Contains(AbilityType))
		{
			CurrentSave->UnlockedAbilities.Remove(AbilityType);
		}
	}
}

bool USaveSubsystem::IsAbilityPermanentlyUnlocked(EMainCharacterAbilityType AbilityType) const
{
	if (CurrentSave)
	{
		return CurrentSave->UnlockedAbilities.Contains(AbilityType);
	}
	return false;
}

TSet<EMainCharacterAbilityType> USaveSubsystem::GetPermanentlyUnlockedAbilities() const
{
	if (CurrentSave)
	{
		return CurrentSave->UnlockedAbilities;
	}
	return TSet<EMainCharacterAbilityType>();
}