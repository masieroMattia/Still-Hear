#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "IndicatorLayer.generated.h"

class UIndicatorManagerComponent;
class UIndicatorDescriptor;
class UIndicatorSubsystem;
class UCanvasPanel;

// Widget responsible for physically rendering and moving the indicator icons on screen

UCLASS()
class STILLHEAR_API UIndicatorLayer : public UCommonUserWidget
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// The canvas where we will spawn the indicator widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> IndicatorCanvas;

	// Minimum distance (in logical pixels) that a clamped indicator keeps from each screen edge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	float ScreenEdgeMargin = 15.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UIndicatorSubsystem> CachedIndicatorSubsystem;

	// Maps a descriptor data to its spawned physical widget
	UPROPERTY(Transient)
	TMap<TObjectPtr<UIndicatorDescriptor>, TObjectPtr<UUserWidget>> SpawnedWidgets;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
#pragma endregion

};
