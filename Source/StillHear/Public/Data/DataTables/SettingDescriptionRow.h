#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SettingDescriptionRow.generated.h"

USTRUCT(BlueprintType)
struct FSettingDescriptionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FText Description;
};
