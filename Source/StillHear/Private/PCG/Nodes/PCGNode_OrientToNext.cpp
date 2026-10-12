#include "PCG/Nodes/PCGNode_OrientToNext.h"

#include "PCGPoint.h"
#include "PCGContext.h"
#include "Math/RandomStream.h"
#include "Data/PCGPointData.h"
#include "PCG/Actors/PCGSplineSequentialMeshes.h"
#include "PCG/Actors/PCGSplineBase.h"

#pragma region INFO
#if WITH_EDITOR
FText UPCGNode_OrientToNextSettings::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("PCG", "OrientToNext", "Orient to Next Point");
}
#endif
#pragma endregion
	
#pragma region PINS
TArray<FPCGPinProperties> UPCGNode_OrientToNextSettings::InputPinProperties() const
{
	// We can accept any type of data as input, but we will mainly be using the source component of the context to get the master spline properties, so we don't need to specify a specific data type for the input pin
	return { FPCGPinProperties(TEXT("In"), EPCGDataType::Any) }; 
}

TArray<FPCGPinProperties> UPCGNode_OrientToNextSettings::OutputPinProperties() const
{
	return { FPCGPinProperties(TEXT("Out"), EPCGDataType::Point) };
}
#pragma endregion
	
#pragma region LOGIC CREATOR
FPCGElementPtr UPCGNode_OrientToNextSettings::CreateElement() const
{
	return MakeShared<FPCGNode_OrientToNextElement>();
}
#pragma endregion

#pragma region LOGIC EXECUTOR
bool FPCGNode_OrientToNextElement::ExecuteInternal(FPCGContext* Context) const
{
	const UPCGNode_OrientToNextSettings* Settings = Context->GetInputSettings<UPCGNode_OrientToNextSettings>();
    check(Settings);

    bool bIsClosedLoop = false;
    bool bApplyOrientation = true;
    bool bShrinkToFitNextPoint = true;
    
    // Automatically detect if the master spline is a closed loop by checking the owner actor
    if (Context->SourceComponent.IsValid())
        if (const APCGSplineSequentialMeshes* Actor = Cast<APCGSplineSequentialMeshes>(Context->SourceComponent->GetOwner()))
        {
            bIsClosedLoop = Actor->Spline->IsClosedLoop();
            bApplyOrientation = Actor->bApplyOrientation;
            bShrinkToFitNextPoint = Actor->bShrinkToFitNextPoint;
        }
    
    // Get the settings of the node. This will allow us to access the properties of the node and use them in the logic
    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(TEXT("In"));
    for (const FPCGTaggedData& InputData : Inputs) // Loop through all the input data connected to the "In" pin. We can have multiple input data if multiple nodes are connected to the same pin
    {
        const UPCGPointData* InPointData = Cast<UPCGPointData>(InputData.Data);
        if (!InPointData) 
            continue;

        // Create a new point data object for the output and initialize it with the input point data. This will allow us to modify the points without affecting the original input data
        UPCGPointData* OutPointData = NewObject<UPCGPointData>();
        OutPointData->InitializeFromData(InPointData);
        TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();
        OutPoints = InPointData->GetPoints();

        // Loop through all the points and modify their rotation and scale based on the location of the next point in the array. We will also check if the settings of the node allow us to modify the rotation and scale before doing so
        for (int32 i = 0; i < OutPoints.Num(); i++)
        {
            // Determine if we should look at a "next" point based on array bounds or closed loop status
            const bool bHasNextPoint = i < OutPoints.Num() - 1 || bIsClosedLoop;

            if (bHasNextPoint)
            {
                // Use modulo to wrap around safely. If is the last index, NextIndex becomes 0
                const int32 NextIndex = (i + 1) % OutPoints.Num();
                
                FVector CurrentLoc = OutPoints[i].Transform.GetLocation();
                FVector NextLoc = OutPoints[NextIndex].Transform.GetLocation();
                FVector Direction = NextLoc - CurrentLoc;
                
                const float Distance = Direction.Size();
                if (Distance > KINDA_SMALL_NUMBER)
                {
                    if (bApplyOrientation)
                        OutPoints[i].Transform.SetRotation(Direction.Rotation().Quaternion());

                    if (bShrinkToFitNextPoint)
                    {
                        const float DynamicReferenceLength = FMath::Max(OutPoints[i].GetExtents().X * 2.0f, 1.0f); // Get the dynamic reference length based on the extents of the point. This will allow us to have a more accurate scaling based on the actual size of the point
                        const float ScaleX = FMath::Min(Distance / DynamicReferenceLength, 1.0f); // Calculate the new scale on the X axis based on the distance to the next point and the dynamic reference length. We also clamp it to a maximum of 1 to avoid scaling up the point if the next point is further away than the reference length
                        const FVector CurrentScale = OutPoints[i].Transform.GetScale3D();
                        
                        OutPoints[i].Transform.SetScale3D(FVector(ScaleX, CurrentScale.Y, CurrentScale.Z));
                    }
                }
            }
            else if (i > 0)
            {
                if (bApplyOrientation) 
                    OutPoints[i].Transform.SetRotation(OutPoints[i - 1].Transform.GetRotation());
                if (bShrinkToFitNextPoint) 
                    OutPoints[i].Transform.SetScale3D(OutPoints[i - 1].Transform.GetScale3D());
            }
        }

        FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
        Output.Data = OutPointData;
        Output.Pin = TEXT("Out");
    }
    
    return true;
}
#pragma endregion
