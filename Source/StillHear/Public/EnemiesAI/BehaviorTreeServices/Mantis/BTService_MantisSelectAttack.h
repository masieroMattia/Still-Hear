#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_MantisSelectAttack.generated.h"

class UCloseAttackData;
class UMantisShiftData;

/*
 * BT Service that checks the distance to the target and writes the appropriate
 * E_MantisAttackType to the blackboard so decorators can choose which attack to execute.
 * Caches data asset pointers from the ASC on first tick
 */
UCLASS()
class STILLHEAR_API UBTService_MantisSelectAttack : public UBTService
{
	GENERATED_BODY()

#pragma region VARIABLES
private:
        // Cached data asset pointers
        TWeakObjectPtr<const UCloseAttackData> CachedCloseData;
        TWeakObjectPtr<const UMantisShiftData> CachedShiftData;
        
        bool bDataCached = false;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBTService_MantisSelectAttack();
#pragma endregion

#pragma region METHODS
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	void CacheDataAssets(const UBehaviorTreeComponent& OwnerComp);
#pragma endregion
};
