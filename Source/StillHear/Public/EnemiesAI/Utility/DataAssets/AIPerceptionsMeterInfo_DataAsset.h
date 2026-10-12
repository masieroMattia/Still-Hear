#pragma once

#include "CoreMinimal.h"
#include "AIInfo_DataAssetBase.h"
#include "AIPerceptionsMeterInfo_DataAsset.generated.h"

UCLASS()
class STILLHEAR_API UAIPerceptionsMeterInfo_DataAsset : public UAIInfo_DataAssetBase
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	// ===== AWARENESS =====
	// The first meter to fill. Goes from 0 to Max. When full, the AI transitions to SUSPICIOUS and unlocks the Alert meter

	// Maximum value of the Awareness meter. When reached, the AI becomes SUSPICIOUS and the Alert phase begins
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Awareness | Setup")
	float MaxAwarenessValue = 100.0f;

	// How much Awareness is subtracted each decay tick while the meter is passively draining back to zero
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Awareness | Setup")
	float AwarenessDecreaseValue = 3.0f;

	// Interval in seconds between each Awareness decay tick once the decay has started.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Awareness | Setup")
	float AwarenessDecreaseTime = 0.5f;

	// Pause period in seconds after the last stimulus before the Awareness meter starts decaying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Awareness | Setup")
	float AwarenessPauseTime = 3.0f;

	// Awareness gained per continuous-sight tick (every 0.1s) when the target is inside the Narrow cone (directly in front)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Sight")
	float AwarenessIncreaseOnSight_Narrow = 50.0f;

	// Awareness gained per continuous-sight tick when the target is inside the Wide cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Sight")
	float AwarenessIncreaseOnSight_Wide = 25.0f;

	// Awareness gained per continuous-sight tick when the target is inside the Peripheral cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Sight")
	float AwarenessIncreaseOnSight_Peripheral = 12.0f;

	// Awareness gained per continuous-sight tick when the target is behind the AI but very close
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Sight")
	float AwarenessIncreaseOnSight_Backward = 8.0f;

	// Awareness gained per hearing event when the player is walking
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Hearing")
	float AwarenessIncreaseOnHearing_Walk = 3.0f;

	// Awareness gained per hearing event when the player is running
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Hearing")
	float AwarenessIncreaseOnHearing_Run = 8.0f;

	// Awareness gained per hearing event when the player is crouching. Set to 0 to make crouching silent
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Hearing")
	float AwarenessIncreaseOnHearing_Crouch = 0.0f;

	// Awareness gained per hearing event when it is a repeater noise
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Awareness | Hearing")
	float AwarenessIncreaseOnHearing_Repeater = 100.0f;

	// ===== ALERT =====
	// The second meter, unlocked after Awareness is full. Goes from 0 to Max. Only SIGHT and TOUCH can fill it (not hearing)
	// When full via sight->ALERTED. When full via touch->HUNTING

	// Maximum value of the Alert meter. When reached via sight the AI becomes ALERTED; via touch it becomes HUNTING
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alert | Setup")
	float MaxAlertValue = 100.0f;

	// How much Alert is subtracted each decay tick while the meter is passively draining
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alert | Setup")
	float AlertDecreaseValue = 2.f;

	// Interval in seconds between each Alert decay tick once the decay has started.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alert | Setup")
	float AlertDecreaseTime = 0.5f;

	// Pause period in seconds after the last stimulus before the Alert meter starts decaying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alert | Setup")
	float AlertPauseTime = 2.0f;

	// Alert gained per continuous-sight tick when the target is inside the Narrow cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alert | Sight")
	float AlertIncreaseOnSight_Narrow = 60.0f;

	// Alert gained per continuous-sight tick when the target is inside the Wide cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alert | Sight")
	float AlertIncreaseOnSight_Wide = 35.0f;

	// Alert gained per continuous-sight tick when the target is inside the Peripheral cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alert | Sight")
	float AlertIncreaseOnSight_Peripheral = 15.0f;

	// Alert gained per continuous-sight tick when the target is behind the AI but very close
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Alert | Sight")
	float AlertIncreaseOnSight_Backward = 8.0f;
#pragma endregion
};
