#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IndicatorDescriptor.generated.h"

class UInputAction;
class UIndicatorWidgetBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIndicatorUpdated);

UCLASS()
class STILLHEAR_API UIndicatorDescriptor : public UObject
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// The actor in the 3D world we want to track
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	TObjectPtr<AActor> TargetActor;

	// Optional offset
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	FVector WorldOffset = FVector(0.0f, 0.0f, 100.0f);

	// The visual widget to spawn
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	TSubclassOf<UIndicatorWidgetBase> IndicatorWidgetClass;

	// If true, the icon sticks to the screen edge when looking away
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	bool bClampToScreen = true;

	// If true, the indicator is only visible when off-screen
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	bool bShowOnlyOffScreen = false;

	// If true, treats the target as off-screen when occluded by geometry (requires bShowOnlyOffScreen)
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	bool bCheckOcclusion = false;
	
	// The specific Enhanced Input actions required for this interaction
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	TArray<TObjectPtr<UInputAction>> InputActions;

	// Custom separator class for this specific indicator
	UPROPERTY(BlueprintReadWrite, Category = "Indicator")
	TSubclassOf<UUserWidget> SeparatorClass;
#pragma endregion
	
#pragma region EVENTS
	// Fired when the indicator data is updated dynamically
	UPROPERTY(BlueprintAssignable, Category = "Indicator")
	FOnIndicatorUpdated OnIndicatorUpdated;
#pragma endregion
};
