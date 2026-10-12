// Copyright https://github.com/MothCocoon/FlowGame/graphs/contributors

#include "Nodes/Triggers/FlowNode_OnTriggerEnter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_OnTriggerEnter)

UFlowNode_OnTriggerEnter::UFlowNode_OnTriggerEnter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReactOnOverlapping = true;
}
