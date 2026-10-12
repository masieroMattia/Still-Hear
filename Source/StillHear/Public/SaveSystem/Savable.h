#pragma once
#include "UObject/Interface.h"
#include "Savable.generated.h"

/**
* Interface for objects that need to perform actions before saving and after loading.
* Implement this interface in any UObject-derived class that requires custom save/load behavior.
*/

UINTERFACE(BlueprintType)
class USavable : public UInterface
{
public:
	GENERATED_BODY()
};

class ISavable
{
	GENERATED_BODY()
	
public:
	
	// Before serializzation
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	void OnPreSave();

	// After serializzation (es. update UI, anim, material, ecc.)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	void OnPostLoad();
};