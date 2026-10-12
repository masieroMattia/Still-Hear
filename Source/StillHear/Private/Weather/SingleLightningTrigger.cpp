// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/SingleLightningTrigger.h"

#include "Components/BoxComponent.h"
#include "TraceAndCollision/CustomCollision.h"


// Sets default values
ASingleLightningTrigger::ASingleLightningTrigger()
{
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>("TriggerVolume");
	TriggerVolume->SetupAttachment(RootComponent);
	
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->SetCollisionProfileName(TEXT("Custom"));
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Overlap);
}

void ASingleLightningTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TriggerLightning();
	TriggerVolume->SetGenerateOverlapEvents(false);
}

// Called when the game starts or when spawned
void ASingleLightningTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
}

