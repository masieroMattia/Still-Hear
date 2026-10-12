#pragma once

#include "CoreMinimal.h"
#include "Interactions/Components/BaseTriggerComponent.h"
#include "BlueprintTriggerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlueprintTriggerEvent, AActor*, OtherActor, UPrimitiveComponent*, OtherComp);

// Generic trigger component that exposes Enter/Exit events to Blueprint for custom logic
UCLASS(ClassGroup=(Interactions), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UBlueprintTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	// Called when a valid actor enters the trigger volume
	UPROPERTY(BlueprintAssignable, Category = "Trigger|Events")
	FOnBlueprintTriggerEvent OnTriggerEnterEvent;

	// Called when a valid actor exits the trigger volume
	UPROPERTY(BlueprintAssignable, Category = "Trigger|Events")
	FOnBlueprintTriggerEvent OnTriggerExitEvent;
#pragma endregion

#pragma region METHODS
public:
	// Call from Blueprint to count this activation towards MaxTriggerCount
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void ConsumeTrigger();

protected:
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
#pragma endregion
};
