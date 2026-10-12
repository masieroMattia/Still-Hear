// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ChangeSpeedType.generated.h"

enum class E_AISpeedType : uint8;
/**
 * 
 */
UCLASS()
class STILLHEAR_API UChangeSpeedType : public UBTService
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "SpeedType")
	E_AISpeedType SpeedType;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	UChangeSpeedType();
#pragma endregion

#pragma region METHODS
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion 
};
