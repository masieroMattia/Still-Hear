#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Mapping of Blackboard Key Names used in the game.
 * This namespace provides a centralized place to define and manage blackboard key names,
 * making it easier to maintain and avoid hardcoding channel values throughout the codebase.
 */
namespace BlackboardKeyNames
{
#pragma region BASE KEYS
	inline const FName KeyNameCurrentWaypoint = "CurrentWaypoint";
	inline const FName KeyNameWaypointWaitTime = "WaypointWaitTime";
	inline const FName KeyNameHasTouched = "HasTouched";
	inline const FName KeyNameTargetLocation = "TargetLocation";
	inline const FName KeyNameTargetActor = "TargetActor";
	inline const FName KeyNameCurrentStatusTag = "CurrentStatusTag";
	inline const FName KeyNameIsCloseEnoughToTarget = "IsCloseEnoughToTarget";
	inline const FName KeyNameIsOffNavMesh = "IsOffNavMesh";
	inline const FName KeyNameNearestSafeLocation = "NearestSafeLocation";
#pragma endregion 
	
#pragma region WORM KEYS
	inline const FName KeyNameWasBellPlayed = "WasBellPlayed";
	inline const FName KeyNameHasRoared = "HasRoared";
	inline const FName KeyNameIsDiving = "IsDiving";
#pragma endregion 

#pragma region MANTIS KEYS
	inline const FName KeyNameEQSRadius = "EQSRadius";
	inline const FName KeyNameEQSDistanceFromEach = "EQSDistanceFromEach";
	inline const FName KeyNameRandomPatrolLocation = "EQSLocation";
	inline const FName DisturbanceLocation = "DisturbanceLocation";
	inline const FName LastDisturbanceLocation = "LastDisturbanceLocation";
	inline const FName KeyNameStartInvestigatingWaitTime = "StartInvestigatingWaitTime";
	inline const FName KeyNameStartHuntingWaitTime = "StartHuntingWaitTime";
	inline const FName KeyNameAttackType = "AttackType";
#pragma endregion 
}
