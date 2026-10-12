#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "IndicatorWidgetBase.generated.h"

class UIndicatorDescriptor;

UCLASS(Abstract, Blueprintable)
class STILLHEAR_API UIndicatorWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// Holds the data payload injected by the Indicator Layer
	UPROPERTY(BlueprintReadOnly, Category = "Indicator", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UIndicatorDescriptor> Descriptor;
#pragma endregion
};
