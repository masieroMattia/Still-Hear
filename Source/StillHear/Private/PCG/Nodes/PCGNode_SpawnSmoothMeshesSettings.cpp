#include "PCG/Nodes/PCGNode_SpawnSmoothMeshesSettings.h"

#include "PCGPoint.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Engine/StaticMesh.h"
#include "PCGManagedResource.h" 
#include "Engine/CollisionProfile.h"
#include "Components/SplineMeshComponent.h"
#include "PCG/Actors/SplineSmoothConnectedMeshes.h"

#pragma region INFO
#if WITH_EDITOR
FText UPCGNode_SpawnSmoothMeshesSettings::GetDefaultNodeTitle() const
{
    return NSLOCTEXT("PCG", "SpawnSmoothMeshes", "Spawn Smooth Spline Meshes");
}
#endif
#pragma endregion
    
#pragma region PINS
TArray<FPCGPinProperties> UPCGNode_SpawnSmoothMeshesSettings::InputPinProperties() const
{
    // Accept points representing the start of each spline segment
    return { FPCGPinProperties(TEXT("In"), EPCGDataType::Point) }; 
}

TArray<FPCGPinProperties> UPCGNode_SpawnSmoothMeshesSettings::OutputPinProperties() const
{
    return { FPCGPinProperties(TEXT("Out"), EPCGDataType::Point) };
}
#pragma endregion
    
#pragma region LOGIC CREATOR
FPCGElementPtr UPCGNode_SpawnSmoothMeshesSettings::CreateElement() const
{
    return MakeShared<FPCGNode_SpawnSmoothMeshesElement>();
}
#pragma endregion

#pragma region LOGIC EXECUTOR
bool FPCGNode_SpawnSmoothMeshesElement::ExecuteInternal(FPCGContext* Context) const
{
    const UPCGNode_SpawnSmoothMeshesSettings* Settings = Context->GetInputSettings<UPCGNode_SpawnSmoothMeshesSettings>();
    if (!Settings) 
        return true;

    ::ESplineMeshAxis::Type ForwardAxis = ::ESplineMeshAxis::X;
    FVector UpDir = FVector(0.0f, 0.0f, 1.0f);
    bool bIsClosedLoop = false;
    bool bEnableCollision = false;
    FVector2D MeshScale = FVector2D(1.0f, 1.0f);
    float OverlapAmount = 0.0f;
    AActor* TargetActor = nullptr;

    // Retrieve parameters directly from our newly created PCGSmoothSpline actor
    if (Context->SourceComponent.IsValid())
    {
        TargetActor = Context->SourceComponent->GetOwner();
        if (const ASplineSmoothConnectedMeshes* SmoothActor = Cast<ASplineSmoothConnectedMeshes>(TargetActor))
        {
            ForwardAxis = SmoothActor->ForwardAxis;
            UpDir = SmoothActor->UpDirection;
            bEnableCollision = SmoothActor->bEnableCollision;
            MeshScale = SmoothActor->MeshScale;
            OverlapAmount = SmoothActor->OverlapAmount;
            
            // Read the closed loop status directly from the inherited Spline component
            if (SmoothActor->Spline)
                bIsClosedLoop = SmoothActor->Spline->IsClosedLoop();
        }
    }

    if (!TargetActor) 
        return true; 

    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(TEXT("In"));
    for (const FPCGTaggedData& InputData : Inputs)
    {
        const UPCGPointData* InPointData = Cast<UPCGPointData>(InputData.Data);
        if (!InPointData) continue;

        const TArray<FPCGPoint>& Points = InPointData->GetPoints();
        const FPCGMetadataAttribute<FSoftObjectPath>* MeshAttribute = InPointData->Metadata ? InPointData->Metadata->GetConstTypedAttribute<FSoftObjectPath>(Settings->MeshAttributeName) : nullptr;

        for (int32 i = 0; i < Points.Num(); i++)
        {
            // Determine if there is a valid next point to connect to
            const bool bHasNext = i < Points.Num() - 1 || bIsClosedLoop;
            if (!bHasNext) 
                continue;

            // Use modulo to wrap around safely. If it is the last index, NextIndex becomes 0
            const int32 NextIndex = (i + 1) % Points.Num();

            FVector StartPos = Points[i].Transform.GetLocation();
            FVector EndPos = Points[NextIndex].Transform.GetLocation();
            
            // Calculate distance to scale the tangents, ensuring a smooth Bézier curve
            const float SegmentLength = FVector::Distance(StartPos, EndPos);

            // Calculate tangents by multiplying the forward vector by the segment length
            const FVector StartTangent = Points[i].Transform.GetRotation().GetForwardVector() * SegmentLength;
            const FVector EndTangent = Points[NextIndex].Transform.GetRotation().GetForwardVector() * SegmentLength;

            UStaticMesh* MeshToUse = nullptr;
            if (MeshAttribute)
            {
                FSoftObjectPath MeshPath = MeshAttribute->GetValueFromItemKey(Points[i].MetadataEntry);
                if (MeshPath.IsValid())
                    MeshToUse = Cast<UStaticMesh>(MeshPath.TryLoad());
            }

            if (!MeshToUse) 
                continue;

            USplineMeshComponent* SplineMeshComp = NewObject<USplineMeshComponent>(TargetActor);
            SplineMeshComp->SetStaticMesh(MeshToUse);
            SplineMeshComp->SetForwardAxis(ForwardAxis, false);
            SplineMeshComp->SetSplineUpDir(UpDir, false);
            SplineMeshComp->SetMobility(EComponentMobility::Static);
            
            if (OverlapAmount > 0.0f)
            {
                FBox Box = MeshToUse->GetBoundingBox();
                float MinBound = Box.Min.X;
                float MaxBound = Box.Max.X;
                if (ForwardAxis == ::ESplineMeshAxis::Y) { MinBound = Box.Min.Y; MaxBound = Box.Max.Y; }
                else if (ForwardAxis == ::ESplineMeshAxis::Z) { MinBound = Box.Min.Z; MaxBound = Box.Max.Z; }

                // Safe clamping to avoid inverting the mesh if overlap is set too high
                const float SafeOverlap = FMath::Min(OverlapAmount, (MaxBound - MinBound) * 0.49f);
                SplineMeshComp->SetBoundaryMin(MinBound + SafeOverlap, false);
                SplineMeshComp->SetBoundaryMax(MaxBound - SafeOverlap, false);
            }
            
            if (bEnableCollision)
            {
                SplineMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                SplineMeshComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
            }
            else
            {
                SplineMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            
            // Apply the calculated deform data and scale
            SplineMeshComp->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
            SplineMeshComp->SetStartScale(MeshScale, true);
            SplineMeshComp->SetEndScale(MeshScale, true);
            
            // Register with the engine so it renders
            SplineMeshComp->RegisterComponent();
            TargetActor->AddInstanceComponent(SplineMeshComp);

            // Add as a managed resource so the PCG Framework cleans it up upon graph regeneration
            UPCGManagedComponent* ManagedComponent = NewObject<UPCGManagedComponent>(Context->SourceComponent.Get());
            ManagedComponent->GeneratedComponent = SplineMeshComp;
            Context->SourceComponent->AddToManagedResources(ManagedComponent);
        }
    }

    // Pass the data forward if needed down the line
    Context->OutputData.TaggedData = Context->InputData.GetInputsByPin(TEXT("In"));
    return true;
}
#pragma endregion