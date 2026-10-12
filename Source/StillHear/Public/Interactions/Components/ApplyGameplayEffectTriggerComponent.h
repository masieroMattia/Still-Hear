#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Interactions/Components/BaseTriggerComponent.h"
#include "ApplyGameplayEffectTriggerComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

// Enum to define how the trigger handles the Gameplay Effect
UENUM(BlueprintType)
enum class EEffectTriggerPolicy : uint8
{
	ApplyRemove		UMETA(DisplayName = "Apply on Enter, Remove on Exit"),
	ApplyOnly		UMETA(DisplayName = "Apply on Enter Only"),
	RemoveOnly		UMETA(DisplayName = "Remove on Enter")
};

UCLASS(ClassGroup=(GAS), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UApplyGameplayEffectTriggerComponent : public UBaseTriggerComponent
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	// The Gameplay Effect class to apply
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	// Defines the behavior of this trigger
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	EEffectTriggerPolicy TriggerPolicy = EEffectTriggerPolicy::ApplyRemove;
#pragma endregion
	
#pragma region VARIABLES
	// Map to keep track of overlapping actors and their specific effect handles
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ActiveEffectHandles;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;
	virtual void OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

public:
	virtual void Reset() override;
#pragma endregion
};