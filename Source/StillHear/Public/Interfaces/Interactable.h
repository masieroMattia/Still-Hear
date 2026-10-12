#pragma once

#include "CoreMinimal.h"
#include "Interactable.generated.h"

UINTERFACE()
class STILLHEAR_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

class STILLHEAR_API IInteractable
{
	GENERATED_BODY()
#pragma region UFUNCTIONS
public:	
	virtual void StartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) {}
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) {}
	
	virtual void TriggerInteraction(AActor* Triggerer = nullptr) {}
	virtual bool GetNearestInteractionSpotLocation(const FVector& FromLocation, FVector& OutLocation, FVector& OutDirection) { return false; }
#pragma endregion
};
