#pragma once

#include "CoreMinimal.h"
#include "Interactions/Components/BaseTriggerComponent.h"
#include "CompanionLightTriggerComponent.generated.h"

/**
 * Directional trigger component that enables or disables the companion light 
 * based on the direction the player enters from
 */
UCLASS(ClassGroup=(Interactions), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UCompanionLightTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

public:
	UCompanionLightTriggerComponent();

protected:
	/** 
	 * If true, entering from the Forward direction turns the light ON
	 * If false, entering from the Forward direction turns the light OFF
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Config")
	bool bForwardTurnsOn = true;

	// If true, the trigger will also update the companion's light color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Config")
	bool bUpdateColor = false;
	
	// Color to set when the light is turned ON
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Config", meta = (EditCondition = "bUpdateColor", EditConditionHides))
	FLinearColor LightColor = FLinearColor::White;

	// Intensity to set when the light is turned ON
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Config")
	float LightIntensity = 0.5f;

	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

private:
	void UpdateCompanionLight(AActor* OtherActor) const;
};
