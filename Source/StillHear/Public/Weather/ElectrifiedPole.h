// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElectrifiedPole.generated.h"

UCLASS()
class STILLHEAR_API AElectrifiedPole : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraComponent> ElectricEffect;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	AElectrifiedPole();
#pragma endregion 

#pragma region METHODS
public:
	void StartEffect();
#pragma endregion 
};
