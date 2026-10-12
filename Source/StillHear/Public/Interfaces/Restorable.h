#pragma once

#include "CoreMinimal.h"
#include "Restorable.generated.h"

UINTERFACE()
class STILLHEAR_API URestorable : public UInterface
{
	GENERATED_BODY()
};

class STILLHEAR_API IRestorable
{
	GENERATED_BODY()
#pragma region METHODS
public:
	virtual void Reset() {}
	virtual void SaveCheckpointState() {}
	virtual void ClearCheckpointState() {}
#pragma endregion
};
