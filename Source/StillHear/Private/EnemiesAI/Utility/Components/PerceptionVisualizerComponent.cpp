#include "EnemiesAI/Utility/Components/PerceptionVisualizerComponent.h"

#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemiesAI/Utility/BlackboardKeyNames.h"
#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"

#pragma region CONSTRUCTOR
UPerceptionVisualizerComponent::UPerceptionVisualizerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
}
#pragma endregion
	
#pragma region METHODS
void UPerceptionVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	MantisRef = Cast<AAIMantisCharacter>(GetOwner());
	if (MantisRef)
	{
		DataAssetRef = MantisRef->GetMantisDataAsset();
	}
}

void UPerceptionVisualizerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// In editor, BeginPlay doesn't run, so we need to ensure references are valid
	if (!IsValid(MantisRef))
	{
		MantisRef = Cast<AAIMantisCharacter>(GetOwner());
	}

	if (IsValid(MantisRef) && !IsValid(DataAssetRef))
	{
		DataAssetRef = MantisRef->GetMantisDataAsset();
	}

	if (!IsValid(MantisRef) || !IsValid(DataAssetRef))
		return;

	if (bShowSightDebug) 
		DrawVisionCones();

	if (bShowLineOfSightDebug)
		DrawLineOfSight();

	if (bShowHearingDebug)
		DrawHearingCircles();
}

void UPerceptionVisualizerComponent::DrawVisionCones() const
{
	FVector Location = MantisRef->GetActorLocation();
	if (IsValid(DataAssetRef))
	{
		Location.Z += DataAssetRef->SightHeight;
	}
	const FVector Forward = MantisRef->GetActorForwardVector();

	// Draw Narrow Cone
	DrawConeSection(Location, Forward, DataAssetRef->SightPeripheralHalfAngleDegree_Narrow, DataAssetRef->SightRadius_Narrow, FColor::Red);

	// Draw Wide Cone
	DrawConeSection(Location, Forward, DataAssetRef->SightPeripheralHalfAngleDegree_Wide, DataAssetRef->SightRadius_Wide, FColor::Yellow);

	// Draw Peripheral Cone
	DrawConeSection(Location, Forward, DataAssetRef->SightPeripheralHalfAngleDegree_Peripheral, DataAssetRef->SightRadius_Peripheral, FColor::Green);

	// Draw Backward Radius - Full circle since it's proximity based
	DrawDebugCircle(GetWorld(), Location, DataAssetRef->SightRadius_Backward, 32, FColor::Cyan, false, -1.f, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
}

void UPerceptionVisualizerComponent::DrawLineOfSight() const
{
	FVector Location = MantisRef->GetActorLocation();
	if (IsValid(DataAssetRef))
	{
		Location.Z += DataAssetRef->SightHeight;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (IsValid(PlayerPawn))
	{
		const FVector TargetLocation = PlayerPawn->GetActorLocation();
		const float VerticalDistance = FMath::Abs(TargetLocation.Z - Location.Z);

		FColor LineColor = FColor::Green;
		if (VerticalDistance > DataAssetRef->MaxSightHeightDifference)
		{
			LineColor = FColor::Red;
		}

		// Draw the line of sight line
		DrawDebugLine(GetWorld(), Location, TargetLocation, LineColor, false, -1.f, 0, 3.f);
	}
}

void UPerceptionVisualizerComponent::DrawHearingCircles() const
{
	const FVector Location = MantisRef->GetActorLocation();

	// Draw Walk Hearing Range
	DrawDebugCircle(GetWorld(), Location, DataAssetRef->WalkHearingRange, 64, FColor::Blue, false, -1.f, 0, 1.5f, FVector(0,1,0), FVector(1,0,0));

	// Draw Run Hearing Range
	DrawDebugCircle(GetWorld(), Location, DataAssetRef->RunHearingRange, 64, FColor::Magenta, false, -1.f, 0, 1.5f, FVector(0,1,0), FVector(1,0,0));
}

void UPerceptionVisualizerComponent::DrawConeSection(const FVector& Location, const FVector& Forward, const float AngleDegrees, const float Radius, const FColor& Color) const
{
	// Calculate the left and right directional limits by rotating the forward vector around the Z axis
	const FVector RightLimit = Forward.RotateAngleAxis(AngleDegrees, FVector::UpVector);
	const FVector LeftLimit = Forward.RotateAngleAxis(-AngleDegrees, FVector::UpVector);

	// Draw the side borders of the cone
	DrawDebugLine(GetWorld(), Location, Location + (RightLimit * Radius), Color, false, -1.f, 0, 2.f);
	DrawDebugLine(GetWorld(), Location, Location + (LeftLimit * Radius), Color, false, -1.f, 0, 2.f);

	// Draw the arc connecting the two limits for better visual representation
	const int32 Segments = FMath::Max(4, FMath::RoundToInt(AngleDegrees / 5.0f));
	const float AngleStep = (AngleDegrees * 2.0f) / Segments;
    
	FVector PreviousPoint = Location + (LeftLimit * Radius);
    
	for (int32 i = 1; i <= Segments; ++i)
	{
		const float CurrentAngle = -AngleDegrees + (AngleStep * i);
		const FVector CurrentDir = Forward.RotateAngleAxis(CurrentAngle, FVector::UpVector);
		const FVector CurrentPoint = Location + (CurrentDir * Radius);
        
		DrawDebugLine(GetWorld(), PreviousPoint, CurrentPoint, Color, false, -1.f, 0, 2.f);
		PreviousPoint = CurrentPoint;
	}
}
#pragma endregion
