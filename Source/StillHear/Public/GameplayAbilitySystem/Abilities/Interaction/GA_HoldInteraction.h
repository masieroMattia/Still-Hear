#pragma once

#include "CoreMinimal.h"
#include "GA_InteractionBase.h"
#include "GA_HoldInteraction.generated.h"

UCLASS()
class STILLHEAR_API UGA_HoldInteraction : public UGA_InteractionBase
{
	GENERATED_BODY()
	
#pragma region CONSTRUCTOR
	UGA_HoldInteraction();
#pragma endregion

#pragma region OVERRIDE METHODS
protected:
	virtual void OnInteractionStart() override;
	virtual void OnStopEventReceived(FGameplayEventData Payload) override;
#pragma endregion
};