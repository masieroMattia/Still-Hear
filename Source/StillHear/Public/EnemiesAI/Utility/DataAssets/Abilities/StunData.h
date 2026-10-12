#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StunData.generated.h"

class UCurveFloat;
class UNiagaraSystem;

UCLASS()
class STILLHEAR_API UStunData : public UDataAsset
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "Stun Settings|Visual")
	TObjectPtr<UCurveFloat> DissolveCurve;

	// Curve driving the initial radius growth of the stun effect
	UPROPERTY(EditAnywhere, Category = "Stun Settings|Visual")
	TObjectPtr<UCurveFloat> RadiusGrowthCurve;

	// Maximum radius value the effect can reach
	UPROPERTY(EditAnywhere, Category = "Stun Settings|Visual")
	float MaxRadius = 150.f;

	// Name of the Niagara parameter to drive
	UPROPERTY(EditAnywhere, Category = "Stun Settings|Visual")
	FName RadiusParameterName = TEXT("User.Radius");

	// Name of the material parameter to drive for the dissolve effect
	UPROPERTY(EditAnywhere, Category = "Stun Settings|Visual")
	FName FadeOpacityParameterName = TEXT("FadeOpacity");

	UPROPERTY(EditAnywhere, Category = "Stun Settings|Collision")
	TArray<TEnumAsByte<ECollisionChannel>> ChannelsToIgnore;
#pragma endregion
};
