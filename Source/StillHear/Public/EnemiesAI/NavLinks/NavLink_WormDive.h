// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLink_WormDive.generated.h"

UCLASS()
class STILLHEAR_API ANavLink_WormDive : public ANavLinkProxy
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<class UGameplayAbility> WormDolphinDiveAbilityClass;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	ANavLink_WormDive();
#pragma endregion 

#pragma region UFUNCTION
private:
	UFUNCTION()
	void NotifyActor(AActor* MovingActor, const FVector& DestinationPoint);
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
#pragma endregion 
};
