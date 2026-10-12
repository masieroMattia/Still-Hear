// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MappingContextList.generated.h"

/**
 * 
 */
UCLASS()
class STILLHEAR_API UMappingContextList : public UDataAsset
{
	GENERATED_BODY()

#pragma region UPROPERTY
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<class UInputMappingContext>> MappingContexts;
#pragma endregion 
};
