#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PushSpotComponent.generated.h"

UCLASS(ClassGroup = "Interactable", meta = (BlueprintSpawnableComponent), DisplayName = "Push Spot")
class STILLHEAR_API UPushSpotComponent : public USceneComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif
	// Name of the socket on the parent Static Mesh for the left hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Sockets")
	FName LeftHandSocketName;
	// Name of the socket on the parent Static Mesh for the right hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Sockets")
	FName RightHandSocketName;
#pragma endregion
	
#pragma region CONSTRUCTOR
	UPushSpotComponent();
#pragma endregion
	
#pragma region METHODS
public:
	// Returns the world transform of the left hand socket directly from the mesh
	FTransform GetLeftHandTransform() const;
	// Returns the world transform of the right hand socket directly from the mesh
	FTransform GetRightHandTransform() const;
#pragma endregion
};
