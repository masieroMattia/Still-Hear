#include "EnemiesAI/EQS/Context/EnvQueryContext_DisturbanceLocation.h"

#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UEnvQueryContext_DisturbanceLocation::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!IsValid(QueryOwner)) 
		return;

	const UBlackboardComponent* BlackboardComp = UAIBlueprintHelperLibrary::GetBlackboard(QueryOwner);
	if (!IsValid(BlackboardComp)) 
		return;

	// If DisturbanceLocation is set on the Blackboard, use it; otherwise fall back to the actor's location
	FVector ResultLocation;
	if (BlackboardComp->IsVectorValueSet(BlackboardKeyNames::DisturbanceLocation))
		ResultLocation = BlackboardComp->GetValueAsVector(BlackboardKeyNames::DisturbanceLocation);
	else
		ResultLocation = QueryOwner->GetActorLocation();

	UEnvQueryItemType_Point::SetContextHelper(ContextData, ResultLocation);
}

