#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "BlobShadowComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UDecalComponent;

UENUM(BlueprintType)
enum class EBlobShadowMode : uint8
{
	FootShadowsOnly UMETA(DisplayName = "Foot Shadows Only"),
	TrailOnly       UMETA(DisplayName = "Trail Only"),
	Both            UMETA(DisplayName = "Both")
};

USTRUCT(BlueprintType)
struct FBlobShadowFoot
{
	GENERATED_BODY()

	// Name of the socket/bone for this foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot")
	FName SocketName = NAME_None;

	// Distance moved between spawning trail decals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot|Trail", meta = (ClampMin = "1.0"))
	float TrailSpawnDistance = 50.0f;

	// How long each trail decal lasts before disappearing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot|Trail", meta = (ClampMin = "0.1"))
	float TrailLifespan = 2.0f;

	// Size of the trail decals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot|Trail", meta = (ClampMin = "0.0"))
	float TrailDecalSize = 48.0f;

	// The decal component used to render the shadow
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot")
	TObjectPtr<UDecalComponent> ShadowDecal = nullptr;

	// Dynamic material instance for controlling opacity at runtime
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;

	// Original source material (used for trail spawning, unaffected by dynamic opacity)
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> SourceMaterial = nullptr;

	// Tracked location of the last trail spawn
	UPROPERTY(Transient)
	FVector LastTrailSpawnLocation = FVector::ZeroVector;

	// Interpolated opacity multiplier for the shadow
	UPROPERTY(Transient)
	float ShadowOpacity = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UBlobShadowComponent : public USceneComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// The left foot configuration and runtime data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Left Foot")
	FBlobShadowFoot LeftFoot;

	// The right foot configuration and runtime data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Right Foot")
	FBlobShadowFoot RightFoot;

	// The operational mode of the blob shadow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
	EBlobShadowMode ShadowMode = EBlobShadowMode::Both;

	// Spacing between feet when sockets are not available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.0"))
	float FootSpacing = 20.0f;

	// Rotation offset (in degrees) to align the decal texture forward direction with the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
	float DecalRotationOffset = 90.0f;

	// Maximum distance to trace for the ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.0"))
	float MaxTraceDistance = 1000.0f;

	// Offset from the ground to prevent Z-fighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
	float GroundOffset = 2.0f;

	// Trace channel for floor detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
	TEnumAsByte<ECollisionChannel> FloorTraceChannel = ECC_Visibility;

	// Base size of the shadow when close to the ground (radius in Y/Z)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.0", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float ShadowBaseSize = 64.0f;

	// Depth of the decal projection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.0", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float ShadowProjectionDepth = 128.0f;

	// How much the shadow shrinks as it gets further from the ground (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float SizeFalloff = 0.4f;

	// Distance at which the shadow reaches its minimum size (SizeFalloff)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "1.0", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float ShrinkDistance = 400.0f;

	// Exponent to make the shrink effect more or less pronounced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.1", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float ShrinkExponent = 1.5f;

	// Speed at which the shadow fades/shrinks when appearing or disappearing (e.g., when jumping)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (ClampMin = "0.1", EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	float ShadowFadeSpeed = 3.0f;

	// Name of the scalar parameter in the decal material used to control opacity from C++
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow", meta = (EditCondition = "ShadowMode != EBlobShadowMode::TrailOnly", EditConditionHides))
	FName OpacityParameterName = FName("Opacity");
#pragma endregion

#pragma region VARIABLES
private:
	// Direct subobject references to allow UE serialization to correctly redirect attachments and avoid CDO/Template mismatches
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shadow", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDecalComponent> LeftShadowDecalComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shadow", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDecalComponent> RightShadowDecalComp = nullptr;

	// Cached owner reference to avoid repeatedly calling GetOwner() in Tick
	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedOwner = nullptr;

	// Cached skeletal mesh component to avoid casting and searching in Tick
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedMesh = nullptr;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UBlobShadowComponent();
#pragma endregion

#pragma region UFUNCTIONS
public:
	// Toggles the trail functionality
	UFUNCTION(BlueprintCallable, Category = "Shadow")
	void SetTrailEnabled(const bool bEnable) { ShadowMode = bEnable ? EBlobShadowMode::Both : EBlobShadowMode::FootShadowsOnly; }
#pragma endregion

#pragma region METHODS

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Initializes starting location and dynamic material for a single foot
	void InitializeFoot(FBlobShadowFoot& Foot, float SideMultiplier);
	// Updates the shadow position and scale based on distance to ground
	void UpdateShadow(float DeltaTime);
	// Updates the shadow and trail for a single foot
	void UpdateFootShadow(FBlobShadowFoot& Foot, float SideMultiplier, float DeltaTime) const;
	// Spawns a trail decal at the current ground position with the given size, trail footprint size, lifespan, and material
	void SpawnTrailDecal(const FHitResult& GroundHit, float Size, float TrailSize, float Lifespan, UMaterialInterface* DecalMat) const;
	// Helper to trace down from a foot location
	bool TraceFootGround(const FVector& FootLoc, FHitResult& OutHit) const;
#pragma endregion
};
