#include "PCG/Nodes/PCGNode_CustomBase.h"

#pragma region INFO
#if WITH_EDITOR
EPCGSettingsType UPCGNode_CustomBaseSettings::GetType() const
{
	return EPCGSettingsType::Spatial;
}

FLinearColor UPCGNode_CustomBaseSettings::GetNodeTitleColor() const
{
	return NodeTitleColor;
}
#endif
#pragma endregion