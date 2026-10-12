#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AIInfo_DataAssetBase.generated.h"

UCLASS()
class STILLHEAR_API UAIInfo_DataAssetBase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 450.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RotationSpeed = 4.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float StunDuration = 2.0f;
};
