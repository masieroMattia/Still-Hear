#pragma once

#include "CoreMinimal.h"
#include "Interactions/Components/BaseTriggerComponent.h"
#include "IndicatorTriggerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UIndicatorTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

public:
	// Indicates if the list is automatically populated. If true, manual editing is disabled.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Indicator Trigger")
	bool bManagedExternally = false;

	// Actors whose IndicatorComponents will be manipulated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Indicator Trigger", meta = (EditCondition = "!bManagedExternally", EditConditionHides))
	TArray<TSoftObjectPtr<AActor>> TargetActors;

	// Should the indicators be activated on enter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Indicator Trigger")
	bool bActivateOnEnter = true;

	// Should the indicators be deactivated on exit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Indicator Trigger")
	bool bDeactivateOnExit = false;

protected:
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

private:
	void ApplyToAllIndicators(const bool bRegister);
};

