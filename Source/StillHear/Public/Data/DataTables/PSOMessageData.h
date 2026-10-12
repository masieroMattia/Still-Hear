#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PSOMessageData.generated.h"

USTRUCT(BlueprintType)
struct FPSOMessageData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;
};
