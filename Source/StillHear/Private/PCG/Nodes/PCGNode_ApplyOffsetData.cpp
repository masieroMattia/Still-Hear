#include "PCG/Nodes/PCGNode_ApplyOffsetData.h"

#include "PCGPoint.h"
#include "PCGContext.h"
#include "Math/RandomStream.h"
#include "Data/PCGPointData.h"
#include "PCG/Actors/PCGSplineSequentialMeshes.h"

#pragma region INFO
#if WITH_EDITOR
FText UPCGNode_ApplyOffsetDataSettings::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("PCG", "ApplyActorData", "Apply Actor Offsets");
}
#endif
#pragma endregion
	
#pragma region PINS
TArray<FPCGPinProperties> UPCGNode_ApplyOffsetDataSettings::InputPinProperties() const
{
	// We can accept any type of data as input, but we will mainly be using the source component of the context to get the master spline properties, so we don't need to specify a specific data type for the input pin
	return { FPCGPinProperties(TEXT("In"), EPCGDataType::Any) }; 
}

TArray<FPCGPinProperties> UPCGNode_ApplyOffsetDataSettings::OutputPinProperties() const
{
	return { FPCGPinProperties(TEXT("Out"), EPCGDataType::Point) };
}
#pragma endregion
	
#pragma region LOGIC CREATOR
FPCGElementPtr UPCGNode_ApplyOffsetDataSettings::CreateElement() const
{
	return MakeShared<UPCGNode_ApplyOffsetDataElement>();
}
#pragma endregion

#pragma region LOGIC EXECUTOR
bool UPCGNode_ApplyOffsetDataElement::ExecuteInternal(FPCGContext* Context) const
{
	FVector LocOffset = FVector::ZeroVector;
    FRotator RotOffset = FRotator::ZeroRotator;
    FVector RandomRotationConfig = FVector::ZeroVector;

    if (Context->SourceComponent.IsValid())
    {
        if (const APCGSplineSequentialMeshes* Actor = Cast<APCGSplineSequentialMeshes>(Context->SourceComponent->GetOwner()))
        {	
            LocOffset = Actor->LocationOffset;
            RotOffset = Actor->RotationOffset;
            RandomRotationConfig = Actor->RandomRotation;
        }
    }

    const FRandomStream RandomStream(Context->SourceComponent->Seed); // Initialize the random stream with the seed of the source component to ensure consistent random values across executions

	// Get the input data from the context and loop through it. We will apply the location and rotation offsets to each point in the input data and then create a new output data with the modified points
    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(TEXT("In"));
    for (const FPCGTaggedData& InputData : Inputs)
    {
        const UPCGPointData* InPointData = Cast<UPCGPointData>(InputData.Data);
        if (!InPointData) continue;

        UPCGPointData* OutPointData = NewObject<UPCGPointData>();
        OutPointData->InitializeFromData(InPointData);
        TArray<FPCGPoint>& OutPoints = OutPointData->GetMutablePoints();
        OutPoints = InPointData->GetPoints();

        for (FPCGPoint& Point : OutPoints)
        {
            FRotator BaseRotator = Point.Transform.Rotator();
            
            FVector LocalOffsetVector = BaseRotator.RotateVector(LocOffset);
            Point.Transform.SetLocation(Point.Transform.GetLocation() + LocalOffsetVector);

            FRotator FinalRotation = BaseRotator + RotOffset;

            const float HalfRoll = RandomRotationConfig.X * 0.5f;
            const float HalfPitch = RandomRotationConfig.Y * 0.5f;
            const float HalfYaw = RandomRotationConfig.Z * 0.5f;

            const FRotator RandomRot(
                RandomStream.FRandRange(-HalfPitch, HalfPitch), 
                RandomStream.FRandRange(-HalfYaw, HalfYaw), 
                RandomStream.FRandRange(-HalfRoll, HalfRoll)
            );
            
            FinalRotation += RandomRot;
            Point.Transform.SetRotation(FinalRotation.Quaternion());
        }

        FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
        Output.Data = OutPointData;
        Output.Pin = TEXT("Out");
    }

    return true;
}
#pragma endregion
