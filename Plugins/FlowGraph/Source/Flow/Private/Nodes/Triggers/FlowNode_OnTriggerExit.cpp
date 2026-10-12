// Copyright https://github.com/MothCocoon/FlowGame/graphs/contributors

#include "Nodes/Triggers/FlowNode_OnTriggerExit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlowNode_OnTriggerExit)

UFlowNode_OnTriggerExit::UFlowNode_OnTriggerExit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReactOnOverlapping = false;
}
