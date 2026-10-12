#pragma once

#include "CoreMinimal.h"
#include "SaveTypes.generated.h"

/** Structs to hold save data
 * Collection of structs to hold save data for actors and levels
 */

USTRUCT(BlueprintType)
struct FComponentSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint8> Bytes;
};

USTRUCT(BlueprintType)
struct FActorSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> Bytes;
	
	UPROPERTY()
	TMap<FName, FComponentSaveData> ComponentsBytes;
};

USTRUCT(BlueprintType)
struct FLevelSaveData
{
	GENERATED_BODY()

	// Actor GUID->Data
	UPROPERTY()
	TMap<FGuid, FActorSaveData> Actors;
};
