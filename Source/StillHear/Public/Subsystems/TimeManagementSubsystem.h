#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimeManagementSubsystem.generated.h"

UCLASS()
class STILLHEAR_API UTimeManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
#pragma region VARIABLES
private:
	UPROPERTY()
	TObjectPtr<UCurveFloat> ActiveTimeCurve;
	FTSTicker::FDelegateHandle TimeTickerHandle;
	FTimerHandle HitStopTimerHandle;
    
	float ElapsedCurveTime = 0.0f;
	float PreviousTimeDilation = 1.0f;
	bool bIsHitStopActive = false;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	// Starts a time dilation curve
	UFUNCTION(BlueprintCallable, Category = "Time Management")
	void PlayTimeCurve(UCurveFloat* Curve);

	// Freezes the game for a fraction of a second to add weight to impacts
	UFUNCTION(BlueprintCallable, Category = "Time Management")
	void TriggerHitStop(float Duration);

	// Resets the global time dilation to normal
	UFUNCTION(BlueprintCallable, Category = "Time Management")
	void ResetTimeDilation();
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void Deinitialize() override;

private:
	// Ticks the time curve over real-world time
	bool TickTimeCurve(float DeltaTime);
    
	// Called when the Hit Stop timer finishes
	void EndHitStop();
#pragma endregion
	
};
