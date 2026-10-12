#pragma once

#include "CoreMinimal.h"
#include "Targetable.generated.h"

UINTERFACE()
class STILLHEAR_API UTargetable : public UInterface
{
	GENERATED_BODY()
};

class STILLHEAR_API ITargetable
{
	GENERATED_BODY()
	
#pragma region UFUNCTIONS
public:
	virtual void SetTargeted() {}
	virtual void SetUntargeted() {}
	
	virtual void HitTarget(int32 DeflectionsCount = 0) {}
	virtual void StopHitTarget() {}
	virtual bool IsTargetable() const { return true; }
#pragma endregion
};
