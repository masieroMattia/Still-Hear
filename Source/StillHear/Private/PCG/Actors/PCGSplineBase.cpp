#include "PCG/Actors/PCGSplineBase.h"

APCGSplineBase::APCGSplineBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);
	
	PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
}