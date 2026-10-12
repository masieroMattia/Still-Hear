#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PSOBlueprintLibrary.generated.h"

UCLASS()
class STILLHEAR_API UPSOBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PSO")
	static int32 GetPSORemaining();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PSO")
	static bool IsPSOCompilationComplete();
};
