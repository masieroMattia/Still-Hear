#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StillHearGameInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnRequestWorldReset);
DECLARE_MULTICAST_DELEGATE(FOnCheckpointSnapshot);
DECLARE_MULTICAST_DELEGATE(FOnClearCheckpointState);

UCLASS()
class STILLHEAR_API UStillHearGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	FOnRequestWorldReset OnRequestWorldReset;
	FOnCheckpointSnapshot OnCheckpointSnapshot;
	FOnClearCheckpointState OnClearCheckpointState;

	bool bIsNewGameResetting = false;

	// True when the character about to spawn belongs to a new game
	bool bIsSpawningNewGame = false;

	// Slot index for a New Game that should be picked up right after the persistent map finishes reloading
	// -1 means no New Game is pending
	int32 PendingNewGameSlotIndex = -1;

	// Same hand-off as PendingNewGameSlotIndex, but for loading an existing save
	// -1 means no Load Game is pending
	int32 PendingLoadGameSlotIndex = -1;

#pragma region UPROPERTY
private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<class UMappingContextList> MappingContextListDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Glyph Data")
	TSubclassOf<class UCommonInputBaseControllerData> KeyboardData;
	UPROPERTY(EditDefaultsOnly, Category = "Glyph Data")
	TSubclassOf<class UCommonInputBaseControllerData> XboxData;
	UPROPERTY(EditDefaultsOnly, Category = "Glyph Data")
	TSubclassOf<class UCommonInputBaseControllerData> PlaystationData;
#pragma endregion

#pragma region METHODS
public:
	TSoftObjectPtr<UMappingContextList> GetMappingContextListDataAsset() { return MappingContextListDataAsset; }
	TSubclassOf<UCommonInputBaseControllerData> GetKeyboardData() { return KeyboardData; }
	TSubclassOf<UCommonInputBaseControllerData> GetXboxData() { return XboxData; }
	TSubclassOf<UCommonInputBaseControllerData> GetPlaystationData() { return PlaystationData; }
	
	virtual void Init() override;
#pragma endregion 
};
