#include "PCG/Nodes/PCGNode_GenerateSplinePoints.h"

#include "PCGPoint.h"
#include "PCGContext.h"
#include "Math/RandomStream.h"
#include "Data/PCGPointData.h"
#include "PCG/Actors/PCGSplineBase.h"

#pragma region INFO
#if WITH_EDITOR
FText UPCGNode_GenerateSplinePointsSettings::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("PCG", "GenerateSplinePoints", "Generate Spline Points");
}
#endif
#pragma endregion
	
#pragma region PINS
TArray<FPCGPinProperties> UPCGNode_GenerateSplinePointsSettings::InputPinProperties() const
{
	// We can accept any type of data as input, but we will mainly be using the source component of the context to get the master spline properties, so we don't need to specify a specific data type for the input pin
	return { FPCGPinProperties(TEXT("In"), EPCGDataType::Any) }; 
}

TArray<FPCGPinProperties> UPCGNode_GenerateSplinePointsSettings::OutputPinProperties() const
{
	return { FPCGPinProperties(TEXT("Out"), EPCGDataType::Point) };
}
#pragma endregion
	
#pragma region LOGIC CREATOR
FPCGElementPtr UPCGNode_GenerateSplinePointsSettings::CreateElement() const
{
	return MakeShared<FPCGNode_GenerateSplinePointsElement>();
}
#pragma endregion

#pragma region LOGIC EXECUTOR
bool FPCGNode_GenerateSplinePointsElement::ExecuteInternal(FPCGContext* Context) const
{
	// Get the settings of the node. This will allow us to access the properties of the node and use them in the logic
	const UPCGNode_GenerateSplinePointsSettings* Settings = Context->GetInputSettings<UPCGNode_GenerateSplinePointsSettings>();
	check(Settings); // Ensure that the settings are valid
	
	float Dist = 100.0f;
	TArray<FSoftObjectPath> MeshPaths;
	const USplineComponent* SplineComp = nullptr;
	
	// Check if the source component of the context is valid and if it is a master spline.
	// If it is, get the properties of the master spline for the logic of this node
	if (Context->SourceComponent.IsValid())
	{
		if (const APCGSplineBase* Actor = Cast<APCGSplineBase>(Context->SourceComponent->GetOwner()))
		{
			Dist = FMath::Max(Actor->Distance, 1.0f);
			SplineComp = Actor->Spline;
			
			for (UStaticMesh* SplineMesh : Actor->Meshes)
			{
				if (SplineMesh)
					MeshPaths.Add(FSoftObjectPath(SplineMesh));
			}
		}
	}

	if (!SplineComp)
		return true; // If there is no valid master spline, we cannot execute the logic of this node, so we return false to indicate that execution was not successful
	
	const FRandomStream RandomStream(Context->SourceComponent->Seed); // Create a random stream with the seed of the context to ensure that the randomization is consistent across executions
	
	// Create a Point container
	UPCGPointData* OutPointData = NewObject<UPCGPointData>();
	TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();
	check(OutPointData->Metadata); 
	
	FPCGMetadataAttribute<FSoftObjectPath> *MeshAttribute = OutPointData->Metadata->CreateAttribute<FSoftObjectPath>(Settings->MeshAttributeName, FSoftObjectPath(), true, true);
	
	// Calculate the total length and how many points we need to generate
	const float SplineLength = SplineComp->GetSplineLength();
	const int32 NumPoints = FMath::Max(1, FMath::FloorToInt(SplineLength / Dist));
	
	for (int32 i = 0; i <= NumPoints; i++)
	{
		const float CurrentDistOnSpline = i * Dist;
		if (CurrentDistOnSpline >= SplineLength && i > 0) // If we reached the end and there is no more space, break the loop
			break;
		
		
		FPCGPoint& NewPoint = OutPoints.AddDefaulted_GetRef(); // Add a new empty point to the array
		OutPointData->Metadata->InitializeOnSet(NewPoint.MetadataEntry); // Assign a Random Mesh
		
		if (MeshPaths.Num() > 0)
		{
			const int32 RandomIndex = RandomStream.RandRange(0, MeshPaths.Num() - 1);
			MeshAttribute->SetValue(NewPoint.MetadataEntry, MeshPaths[RandomIndex]);
		}
		
		FVector CurrentLocation = SplineComp->GetLocationAtDistanceAlongSpline(CurrentDistOnSpline, ESplineCoordinateSpace::World);
		FRotator SplineRotation = SplineComp->GetRotationAtDistanceAlongSpline(CurrentDistOnSpline, ESplineCoordinateSpace::World);
        
		NewPoint.SetExtents(FVector(Dist * 0.5f)); // Set the extents of the point to be half of the distance between points. This will allow us to have a better visualization of the points and also to have a more accurate collision if we use the points for spawning actors
		
		// Apply calculations to the new point
		NewPoint.Transform.SetLocation(CurrentLocation);
		NewPoint.Transform.SetRotation(SplineRotation.Quaternion());
		NewPoint.Transform.SetScale3D(FVector(1.0f));
	}
	
	FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
	Output.Data = OutPointData;
	Output.Pin = TEXT("Out");
	
	return true;
}
#pragma endregion