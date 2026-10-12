// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/ElectrifiedPole.h"

#include "NiagaraComponent.h"

AElectrifiedPole::AElectrifiedPole()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);

	ElectricEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ElectricEffect"));
	ElectricEffect->SetupAttachment(StaticMeshComponent);
}

void AElectrifiedPole::StartEffect()
{
	ElectricEffect->Activate(true);
}

