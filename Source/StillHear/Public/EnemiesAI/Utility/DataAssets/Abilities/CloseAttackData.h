#pragma once

#include "CoreMinimal.h"
#include "AttackBaseData.h"
#include "CloseAttackData.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class STILLHEAR_API UCloseAttackData : public UAttackBaseData
{
	GENERATED_BODY()

public:
	// Maximum distance from the target to trigger this attack (used by the BT)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Close Attack", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CloseAttackDistance = 250.0f;
	// Montage played for the close-range melee attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Close Attack")
	TObjectPtr<UAnimMontage> CloseAttackMontage;

	// How far ahead in time we predict the target's location
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Close Attack|Prediction", meta = (ClampMin = "0.0"))
	float PredictionTimeDelay = 0.5f;
	// Multiplier for the target's velocity during prediction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Close Attack|Prediction", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float PredictionFactor = 1.0f;
	
	// Speed at which the enemy tracks the target during the attack montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Close Attack|Prediction", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TrackingRotationSpeed = 300.0f;
};
