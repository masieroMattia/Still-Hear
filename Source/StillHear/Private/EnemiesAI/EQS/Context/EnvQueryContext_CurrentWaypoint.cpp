#include "EnemiesAI/EQS/Context/EnvQueryContext_CurrentWaypoint.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_CurrentWaypoint::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!IsValid(QueryOwner)) return;

	const UBlackboardComponent* BlackboardComp = UAIBlueprintHelperLibrary::GetBlackboard(QueryOwner);
	if (!IsValid(BlackboardComp)) return;

	// Get the Waypoint from the Blackboard
	const AActor* CurrentWaypoint = Cast<AActor>(BlackboardComp->GetValueAsObject(BlackboardKeyNames::KeyNameCurrentWaypoint));
    
	// Tell EQS to use this Waypoint as the center of the grid
	if (IsValid(CurrentWaypoint))
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, CurrentWaypoint);
}
