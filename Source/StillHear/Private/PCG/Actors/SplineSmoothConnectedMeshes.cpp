#include "PCG/Actors/SplineSmoothConnectedMeshes.h"

ASplineSmoothConnectedMeshes::ASplineSmoothConnectedMeshes()
{
	// Set safe defaults for spline mesh orientation
	ForwardAxis = ESplineMeshAxis::X;
	UpDirection = FVector(0.0f, 0.0f, 1.0f);
}