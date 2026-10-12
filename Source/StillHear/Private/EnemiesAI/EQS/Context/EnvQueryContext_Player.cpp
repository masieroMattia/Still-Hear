#include "EnemiesAI/EQS/Context/EnvQueryContext_Player.h"

#include "Kismet/GameplayStatics.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner) 
		return;

	const UWorld* World = QueryOwner->GetWorld();
	if (!World) 
		return;

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!IsValid(PlayerPawn)) 
		return;

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerPawn);
}

