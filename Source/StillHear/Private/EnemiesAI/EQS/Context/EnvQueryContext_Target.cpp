// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/EQS/Context/EnvQueryContext_Target.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();

	if (!QueryOwner)
		return;

	const AStillHearAICharacterBase* AICharacter = Cast<AStillHearAICharacterBase>(QueryOwner);

	if (!AICharacter)
		return;

	AStillHearAIControllerBase* Controller = AICharacter->GetAICRef();

	if (!Controller)
		return;

	const UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();

	if (!Blackboard)
		return;

	const FVector Target = Blackboard->GetValueAsVector(BlackboardKeyNames::KeyNameTargetLocation);
	UEnvQueryItemType_Point::SetContextHelper(ContextData, Target);
}
