#pragma once

#include "CoreMinimal.h"
#include "TargetableDrivableObj.h"
#include "SequenceObj.generated.h"

UCLASS()
class STILLHEAR_API ASequenceObj : public ATargetableDrivableObj
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	int RequiredDeflections = 3; // Number of deflections required to trigger the interaction
#pragma endregion
	
#pragma region OVERRIDE METHODS
protected:
	virtual bool ShouldActivateOnHit(int32 DeflectionsCount) const override;
#pragma endregion
};