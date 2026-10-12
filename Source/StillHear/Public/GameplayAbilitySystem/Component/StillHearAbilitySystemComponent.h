// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StillHearAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UStillHearAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

#pragma region VARIABLES
protected:
	/*
	 * Here I keep the list of abilities that I have. Thanks to this, I can check if my abilities changed
	 * (if they have been added or removed) by comparing this array with the array of the base class ActivatableAbilities,
	 * which is automatically updated instead
	 */
	TArray<FGameplayAbilitySpec> LastActivatableAbility;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	UStillHearAbilitySystemComponent();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;

	/*
	 * Here I check if my abilities changed or not when I give or remove abilities, and in that case, I send an event
	 * that could be needed by someone (for example the UI to update some widgets)
	 */
	virtual void OnRep_ActivateAbilities() override;
#pragma endregion

	





};
