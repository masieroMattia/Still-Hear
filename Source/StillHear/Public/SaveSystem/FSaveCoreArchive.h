#pragma once

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

/**
 * Custom archive for saving/loading game data
 */
struct FSaveCoreArchive : public FObjectAndNameAsStringProxyArchive
{
	explicit FSaveCoreArchive(FArchive& InInnerArchive)
		: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true; // Save only UPROPERTY(SaveGame)
		ArNoDelta = true;    // Set to true serialize all properties, even if they have default values
	}
};
