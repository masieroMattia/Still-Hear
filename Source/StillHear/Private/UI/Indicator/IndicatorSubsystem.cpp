#include "UI/Indicator/IndicatorSubsystem.h"

#pragma region METHODS
void UIndicatorSubsystem::AddIndicator(UIndicatorDescriptor* Descriptor)
{
	if (Descriptor && !ActiveIndicators.Contains(Descriptor))
		ActiveIndicators.Add(Descriptor);
}

void UIndicatorSubsystem::RemoveIndicator(UIndicatorDescriptor* Descriptor)
{
	if (Descriptor)
		ActiveIndicators.Remove(Descriptor);
}
#pragma endregion