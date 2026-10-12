#include "Interactions/Actors/SequenceObj.h"

#pragma region OVERRIDE METHODS
bool ASequenceObj::ShouldActivateOnHit(const int32 DeflectionsCount) const
{
	return DeflectionsCount >= RequiredDeflections;
}
#pragma endregion