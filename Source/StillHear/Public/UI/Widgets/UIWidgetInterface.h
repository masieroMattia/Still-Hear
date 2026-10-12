#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIWidgetInterface.generated.h"

/**
 * Generic interface for UI widgets to receive data from FlowNodes or other systems.
 */
UINTERFACE(BlueprintType)
class STILLHEAR_API UUIWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class STILLHEAR_API IUIWidgetInterface
{
	GENERATED_BODY()

public:
	/** 
	 * Called to initialize the widget with text, an optional duration, and optional input actions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UI")
	void InitializeWidget(const FText& Text, float Duration, const TArray<UInputAction*>& InputActions);
};
