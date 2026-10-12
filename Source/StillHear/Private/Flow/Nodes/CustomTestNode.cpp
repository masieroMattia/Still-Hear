// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/Nodes/CustomTestNode.h"

UCustomTestNode::UCustomTestNode()
{
	// Setting the category and the style of the node
#if WITH_EDITOR
	Category = TEXT("Custom");
	NodeDisplayStyle = FlowNodeStyle::Node;
#endif

	// Setting the input and output pins
	InputPins.Empty();
	OutputPins.Empty();
	InputPins.Add(FFlowPin(FName("Input")));
	OutputPins.Add(FFlowPin(FName("Output")));

	// Adding an input bool pin as example
	FFlowPin InPin = FFlowPin(FName("InBool"));
	InPin.SetPinType(EFlowPinType::Bool);
	InputPins.Add(InPin);

	// Adding an output bool pin as example
	FFlowPin OutPin = FFlowPin(FName("OutBool"));
	OutPin.SetPinType(EFlowPinType::Bool);
	OutputPins.Add(OutPin);
}

void UCustomTestNode::ExecuteInput(const FName& PinName)
{
	// Analyzing the input that has a default value
	FFlowDataPinResult_Vector VectorResult = TryResolveDataPinAsVector(GET_MEMBER_NAME_CHECKED(UCustomTestNode, InVector));

	if (VectorResult.Result == EFlowDataPinResolveResult::FailedMissingPin)
	{
		// Do something...
	}
	else
	{
		// Do something else...
	}
}

void UCustomTestNode::Cleanup()
{
	Super::Cleanup();
}
