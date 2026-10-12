#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IKTargetReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UIKTargetReceiver : public UInterface
{
	GENERATED_BODY()
};

class STILLHEAR_API IIKTargetReceiver
{
	GENERATED_BODY()

public:
	// Receives world locations for IK target effectors
	virtual void UpdateIKTargets(const FTransform& LeftTarget, const FTransform& RightTarget) = 0;
};