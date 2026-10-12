#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GraphicsDebugWidget.generated.h"

class UTextBlock;

/**
 * Overlay widget that shows real-time graphics/upscaler CVar values
 */
UCLASS()
class STILLHEAR_API UGraphicsDebugWidget : public UCommonUserWidget
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
#pragma endregion

#pragma region UPROPERTIES
protected:
	// The text block that displays debug info. Bind it in UMG with the exact name "DebugText"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> DebugText;
#pragma endregion

#pragma region VARIABLES
private:
	// How often (seconds) to refresh the debug text. 0 = every frame
	UPROPERTY(EditAnywhere, Category="Debug", meta=(ClampMin=0.0f))
	float RefreshInterval = 0.1f;

	float TimeSinceLastRefresh = 0.f;
#pragma endregion

#pragma region METHODS
private:
	void RefreshDebugText();
	static FString GetAAMethodName(int32 Method);
#pragma endregion
};
