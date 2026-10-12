// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "CustomTestNode.generated.h"

/**
 * Add a node description like this when you hover the node with the mouse cursor
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Custom Node"))
class STILLHEAR_API UCustomTestNode : public UFlowNode
{
	GENERATED_BODY()

#pragma region UPROPERTY
	UPROPERTY(EditAnywhere, Category = "Flow", meta = (DefaultForInputFlowPin, FlowPinType = Vector))
	FVector InVector;
	UPROPERTY(VisibleAnywhere, Category = "Flow", meta = (DefaultForOutputFlowPin, FlowPinType = Vector))
	FVector OutVector;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	// Set the category, the style of the node, the input and output pins name and type
	UCustomTestNode();
#pragma endregion

#pragma region METHODS
public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;
#pragma endregion
};
