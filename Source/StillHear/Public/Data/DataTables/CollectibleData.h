#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "CollectibleData.generated.h"

/**
 * Data structure for Collectible items to be used in DataTables
 */ 
 USTRUCT(BlueprintType)
 struct STILLHEAR_API FCollectibleData : public FTableRowBase
 {
	 GENERATED_BODY()
	 
	 // The text/description of the collectible
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible Data")
	 FText CollectibleText;
	 
	 // An optional simple 2D texture for the collectible
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible Data")
	 TSoftObjectPtr<UTexture2D> CollectibleImage;
	 
	 // An optional material (useful for animated UI or special effects)
	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible Data")
	 TSoftObjectPtr<UMaterialInterface> CollectibleMaterial;
};
	
inline FText CollectibleRowNameToDisplayText(FName RowName)
{
	FString Str = RowName.ToString();
	FString Result;
	Result.Reserve(Str.Len() + 8);
	for (int32 i = 0; i < Str.Len(); ++i)
	{
		if (i > 0 && FChar::IsUpper(Str[i]))
			Result.AppendChar(' ');
		Result.AppendChar(Str[i]);
	}
	return FText::FromString(Result);
}