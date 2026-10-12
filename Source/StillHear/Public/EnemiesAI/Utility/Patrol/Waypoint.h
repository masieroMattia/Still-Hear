#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "Waypoint.generated.h"

UCLASS()
class STILLHEAR_API AWaypoint : public ATargetPoint
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<AWaypoint> NextWaypoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	float WaitTime = 0.0f;

	// Snap this waypoint to the floor surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Snap")
	bool bSnapToFloor = false;

	// Additional Z offset added on top of the snapped floor position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Snap",
		meta = (EditCondition = "bSnapToFloor", EditConditionHides))
	float FloorOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Debug")
	bool bShowDebugTrace = false;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	AWaypoint();
#pragma endregion 

#pragma region UFUNCTION
public:
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	AWaypoint* GetNextWaypoint() const { return NextWaypoint; }

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	float GetWaitTime() const { return WaitTime; }
#pragma endregion 
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
#pragma endregion 
};
