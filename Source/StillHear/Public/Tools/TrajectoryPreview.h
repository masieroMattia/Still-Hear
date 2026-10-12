#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Data/DataAssets/SoundWaveData.h"
#include "TrajectoryPreview.generated.h"

UCLASS()
class STILLHEAR_API ATrajectoryPreview : public AActor
{
	GENERATED_BODY()

#pragma region EVENTS
private:
	FDelegateHandle ActorMovedDelegateHandle;
#pragma endregion
	
#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> ArrowComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Data")
	TObjectPtr<USoundWaveData> SoundWaveData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Arrow")
	FColor ArrowColor = FColor::Black;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Arrow")
	float ArrowSize = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Line")
	float LineTraceZOffset = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Line")
	FColor LineColor = FColor::Cyan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Line")
	float LineThickness = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Line")
	int MaxBounces = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Line")
	float MaxDistance = 5000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Point")
	FColor ImpactPointColor = FColor::Yellow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Settings|Point")
	float ImpactPointSize = 15.0f;
#pragma endregion
	
#pragma region VARIABLES
protected:
	float CachedProjectileRadius = 10.0f;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	ATrajectoryPreview();
#pragma endregion 
	
#if WITH_EDITOR
#pragma region METHODS
public:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginDestroy() override; // Ensure to unbind the delegate when the actor is destroyed to prevent dangling references
	
private:
	void OnGlobalActorMoved(AActor* MovedActor) const; // Callback function for the global actor moved event
	void DrawTrajectory() const;
#pragma endregion
#endif
};
