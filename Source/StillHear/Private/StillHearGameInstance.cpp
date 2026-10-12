#include "StillHearGameInstance.h"

#include "AbilitySystemGlobals.h"

#pragma region METHODS
void UStillHearGameInstance::Init()
{
	Super::Init();
	
	UAbilitySystemGlobals::Get().InitGlobalData();
}

#pragma endregion
