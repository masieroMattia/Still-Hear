#include "Interactions/Components/CompanionInteractionSpotComponent.h"

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"

#pragma region CONSTRUCTOR
UCompanionInteractionSpotComponent::UCompanionInteractionSpotComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}
#pragma endregion

#pragma region METHODS

USplineComponent* UCompanionInteractionSpotComponent::GetAssociatedSpline() const
{
    if (!GetOwner())
    {
        return nullptr;
    }

    TArray<USplineComponent*> Splines;
    GetOwner()->GetComponents<USplineComponent>(Splines);

    if (Splines.IsEmpty())
    {
        return nullptr;
    }

    // If a tag is specified, look for a matching spline component
    if (!SplineNameTag.IsNone())
    {
        for (USplineComponent* Spline : Splines)
        {
            if (Spline->GetFName() == SplineNameTag || Spline->ComponentHasTag(SplineNameTag))
            {
                return Spline;
            }
        }
    }

    // Fallback: return the first one found
    return Splines[0];
}
#pragma endregion
