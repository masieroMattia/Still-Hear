#include "GameplayAbilitySystem/Cues/GC_ParryWindow.h"

#include "NiagaraComponent.h"
#include "Data/DataAssets/ParryData.h"

// Define constants
const FName AGC_ParryWindow::RadiusParamName = TEXT("Radius");
const FName AGC_ParryWindow::DurationParamName = TEXT("Duration");

#pragma region CONSTRUCTOR
AGC_ParryWindow::AGC_ParryWindow()
{
	SpawnedOnCharacter = true;
}
#pragma endregion
	
#pragma region METHODS
bool AGC_ParryWindow::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	const bool bResult = Super::OnActive_Implementation(MyTarget, Parameters);

	if (bResult && SpawnedVFX && ParryData)
	{
		SpawnedVFX->SetFloatParameter(RadiusParamName, ParryData->ParryRadius);
		SpawnedVFX->SetFloatParameter(DurationParamName, ParryData->ParryWindowDuration);
	}

	return bResult;
}
#pragma endregion