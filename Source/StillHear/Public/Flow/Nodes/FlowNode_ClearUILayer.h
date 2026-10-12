#pragma once

#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h"
#include "FlowNode_ClearUILayer.generated.h"

/**
 * FlowNode that clears all widgets from a specific UI layer.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Clear UI Layer"))
class STILLHEAR_API UFlowNode_ClearUILayer : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_ClearUILayer();

protected:
	/** The layer to clear (e.g. UI.Layer.Game) */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;

public:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif
};

