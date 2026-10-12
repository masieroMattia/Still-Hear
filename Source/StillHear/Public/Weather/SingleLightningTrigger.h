// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingleLightningBase.h"
#include "SingleLightningTrigger.generated.h"

UCLASS()
class STILLHEAR_API ASingleLightningTrigger : public ASingleLightningBase
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trigger")
	TObjectPtr<class UBoxComponent> TriggerVolume;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	ASingleLightningTrigger();
#pragma endregion 

#pragma region UFUNCTIONS
protected:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
#pragma endregion 
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;
#pragma endregion 
	
};
