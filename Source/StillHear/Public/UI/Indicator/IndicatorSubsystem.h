#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "IndicatorSubsystem.generated.h"

class UIndicatorDescriptor;

UCLASS()
class STILLHEAR_API UIndicatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
	UPROPERTY(Transient)
	TArray<TObjectPtr<UIndicatorDescriptor>> ActiveIndicators;
#pragma endregion
	
#pragma region METHODS
public:
	void AddIndicator(UIndicatorDescriptor* Descriptor);
	void RemoveIndicator(UIndicatorDescriptor* Descriptor);

	const TArray<TObjectPtr<UIndicatorDescriptor>>& GetIndicators() const { return ActiveIndicators; }
#pragma endregion
};
