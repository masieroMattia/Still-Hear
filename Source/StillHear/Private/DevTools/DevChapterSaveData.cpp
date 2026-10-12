#include "DevTools/DevChapterSaveData.h"

#if WITH_EDITOR

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "SaveSystem/SaveSubsystem.h"

void UDevChapterSaveData::CaptureFromPlaySession()
{
	// Find the running PIE world (capture only makes sense while playing).
	UWorld* PIEWorld = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
		{
			if (Ctx.WorldType == EWorldType::PIE && Ctx.World())
			{
				PIEWorld = Ctx.World();
				break;
			}
		}
	}

	if (!PIEWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DevChapterCapture] No PIE session found. Play in editor and reach a checkpoint first."));
		return;
	}

	UGameInstance* GI = PIEWorld->GetGameInstance();
	USaveSubsystem* SaveSub = GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DevChapterCapture] SaveSubsystem not available."));
		return;
	}

	TArray<uint8> Bytes;
	if (!SaveSub->CaptureCurrentSaveToBytes(Bytes) || Bytes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DevChapterCapture] No current save to capture. Cross a saving checkpoint first."));
		return;
	}

	SaveBytes = MoveTemp(Bytes);
	MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("[DevChapterCapture] Captured %d bytes into '%s'. Remember to save the asset."), SaveBytes.Num(), *GetName());
}

#endif
