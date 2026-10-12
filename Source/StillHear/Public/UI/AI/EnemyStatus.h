#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Indicator/IndicatorWidgetBase.h"
#include "EnemyStatus.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
class UTextBlock;
class AStillHearAIControllerBase;

USTRUCT(BlueprintType)
struct FEnemyStatusData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.Texture2D, /Script/Engine.MaterialInterface"))
	TObjectPtr<UObject> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsVisible = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bClampToScreen = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShowOnlyOffScreen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAlwaysFull = false;
};

UCLASS(Abstract)
class STILLHEAR_API UEnemyStatus : public UIndicatorWidgetBase
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StatusIconBar;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI|Status Configuration")
	TMap<FGameplayTag, FEnemyStatusData> StatusConfig;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	TWeakObjectPtr<AStillHearAIControllerBase> CachedAIC;

	const FEnemyStatusData* CachedAlertData;
	const FEnemyStatusData* CachedSuspiciousData;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	
private:
	void UpdateWidgetVisuals(const FEnemyStatusData* Data, float Percent) const;
	const FEnemyStatusData* FindStatusData(const FGameplayTag& Tag) const;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION()
	void UpdateMeter(const float AwarenessPercent, const float AlertPercent);

	UFUNCTION()
	void UpdateStatus(const FGameplayTag NewStatusTag);
#pragma endregion
};
