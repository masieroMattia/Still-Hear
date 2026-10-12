// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/NavLinks/NavLink_WormDive.h"

#include "EnemiesAI/Controllers/Worm/AIWormController.h"
#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"


// Sets default values
ANavLink_WormDive::ANavLink_WormDive()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bSmartLinkIsRelevant = true;
}

void ANavLink_WormDive::NotifyActor(AActor* MovingActor, const FVector& DestinationPoint)
{
	AAIWormCharacter* Character = Cast<AAIWormCharacter>(MovingActor);

	if (!IsValid(Character))
		return;
	
	UAbilitySystemComponent* Asc = Character->GetAbilitySystemComponent();

	if (!IsValid(Asc))
		return;

	AAIWormController* Controller = Cast<AAIWormController>(Character->GetAICRef());

	if (!IsValid(Controller))
		return;

	Controller->SetCurrentTargetLocation(DestinationPoint);
	Asc->TryActivateAbilityByClass(WormDolphinDiveAbilityClass);
}

// Called when the game starts or when spawned
void ANavLink_WormDive::BeginPlay()
{
	Super::BeginPlay();

	OnSmartLinkReached.AddUniqueDynamic(this, &ThisClass::NotifyActor);
}

