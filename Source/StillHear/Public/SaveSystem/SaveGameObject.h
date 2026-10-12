#pragma once

#include "CoreMinimal.h"
#include "FlowSave.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem/SaveTypes.h"
#include "Character/MainCharacterAbilityType.h"
#include "GameplayTagContainer.h"
#include "SaveGameObject.generated.h"

/**
 * This class holds all the save data for the game.
 * Settings are stored in a separate USettingsSaveGame class.
 */
UCLASS()
class STILLHEAR_API USaveGameObject : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	TMap<FName, FLevelSaveData> Levels;

	UPROPERTY()
	FString SlotName;
	
	// --- UI METADATA ---
	// The date and time when the game was saved
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData|UI")
	FDateTime SaveDate;

	// The name of the map the player was in
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData|UI")
	FName LastLevelName;

	// Row name in CheckpointDisplayData table — set when a Flow checkpoint triggers a save
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData|UI")
	FName CheckpointId;

	// Updated every time this slot is loaded — used to determine the most recently played slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData|UI")
	FDateTime LastPlayedDate;
	
	UPROPERTY(VisibleAnywhere, Category = "Flow")
	TArray<FFlowComponentSaveData> FlowComponents;

	UPROPERTY(VisibleAnywhere, Category = "Flow")
	TArray<FFlowAssetSaveData> FlowInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData")
	TSet<EMainCharacterAbilityType> UnlockedAbilities;

	/** The stack of active audio states (for music/ambience persistence) */
	UPROPERTY(VisibleAnywhere, Category = "SaveData|Audio")
	TArray<FGameplayTag> AudioStateStack;
};
