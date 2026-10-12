#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraEffects/CameraEffectTypes.h"
#include "ParryData.generated.h"

class UGameplayEffect;
class UForceFeedbackEffect;
class USoundBase;

UCLASS()
class STILLHEAR_API UParryData : public UDataAsset
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditDefaultsOnly, Category = "Parry Settings|Montage")
	TObjectPtr<UAnimMontage> ParryMontage;
	
	UPROPERTY(EditAnywhere, Category = "Parry Settings|Camera Effects")
	FCameraEffectPreset EffectPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Parry Settings|Config")
	float ParryRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Parry Settings|Config")
	float ParryWindowDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Parry Settings|Config")
	float InvulnerabilityDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Config")
	TSubclassOf<UGameplayEffect> InvulnerabilityEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Config")
	TArray<TEnumAsByte<ECollisionChannel>> DetectionChannels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Config")
	FGameplayTagContainer RequiredEnemyTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, UIMin = 0.0f), Category = "Parry Settings|Config")
	float CooldownDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Sounds")
	TObjectPtr<USoundBase> FinishCooldownSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Stun")
	TSubclassOf<UGameplayEffect> StunEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|SloMo")
	TObjectPtr<UCurveFloat> SloMoEffectCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> ParryPressedForceFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> ParrySuccessForceFeedback;
#pragma endregion
};
