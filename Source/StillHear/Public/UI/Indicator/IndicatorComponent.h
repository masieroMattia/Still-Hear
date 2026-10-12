#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IndicatorComponent.generated.h"

class UInputAction;
class UIndicatorDescriptor;
class UIndicatorWidgetBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	// If true, the indicator will register automatically on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	bool bAutoRegister = true;
	
	// The visual widget to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	TSubclassOf<UIndicatorWidgetBase> IndicatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FVector WorldOffset = FVector(0.0f, 0.0f, 120.0f);

	// If true, the icon sticks to the screen edge when looking away
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	bool bClampToScreen = true;

	// If true, the indicator is only visible when off-screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	bool bShowOnlyOffScreen = false;

	// If true, treats the target as off-screen when occluded by geometry (requires bShowOnlyOffScreen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	bool bCheckOcclusion = false;
	
	// The actions mapped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	TArray<TObjectPtr<UInputAction>> InputActions;

	// Custom separator widget used between inputs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	TSubclassOf<UUserWidget> SeparatorClass;
#pragma endregion
	
#pragma region VARIABLES
private:
	UPROPERTY(Transient)
	TObjectPtr<UIndicatorDescriptor> IndicatorDescriptor;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UIndicatorComponent();
#pragma endregion

#pragma region METHODS
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	void RegisterIndicator();

	UFUNCTION(BlueprintCallable, Category = "Indicator")
	void UnregisterIndicator();

	UFUNCTION(BlueprintCallable, Category = "Indicator")
	void UpdatePromptActions(const TArray<UInputAction*>& NewActions, TSubclassOf<UUserWidget> NewSeparatorClass = nullptr);
#pragma endregion
};
