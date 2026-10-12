#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STILLHEAR_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
#pragma region UPROPERTY
public:
	// Speed Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData BaseSpeed;
	ATTRIBUTE_ACCESSORS(UBasicAttributeSet, BaseSpeed);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData SpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UBasicAttributeSet, SpeedMultiplier);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxParryAngle;
	ATTRIBUTE_ACCESSORS(UBasicAttributeSet, MaxParryAngle);
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	UBasicAttributeSet();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

private:
	void UpdateCharacterSpeed(float Base, float Multiplier) const;
#pragma endregion

	



};
