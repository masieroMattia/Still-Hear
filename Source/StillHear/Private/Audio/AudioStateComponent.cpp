#include "Audio/AudioStateComponent.h"
#include "Audio/StillHearAudioObserver.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

UAudioStateComponent::UAudioStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UAudioStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bAutoDetectOverlap)
	{
		SetComponentTickEnabled(false);
	}

	if (bRequestActive)
	{
		NotifyObserver();
	}
}

void UAudioStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		if (PlayerPawn && GetOwner())
		{
			bool bIsOverlapping = false;
			TArray<UPrimitiveComponent*> PrimComps;
			GetOwner()->GetComponents<UPrimitiveComponent>(PrimComps);
			for (UPrimitiveComponent* PrimComp : PrimComps)
			{
				if (PrimComp && PrimComp->IsOverlappingActor(PlayerPawn))
				{
					bIsOverlapping = true;
					break;
				}
			}

			if (bRequestActive != bIsOverlapping)
			{
				bRequestActive = bIsOverlapping;
				NotifyObserver();
			}
		}
	}
}

void UAudioStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRequestActive)
	{
		bRequestActive = false;
		NotifyObserver();
	}
	Super::EndPlay(EndPlayReason);
}

void UAudioStateComponent::SetAudioStateActive(bool bNewActive)
{
	if (bRequestActive != bNewActive)
	{
		bRequestActive = bNewActive;
		NotifyObserver();
	}
}

void UAudioStateComponent::NotifyObserver()
{
	if (UWorld* World = GetWorld())
	{
		if (UStillHearAudioObserver* Observer = World->GetSubsystem<UStillHearAudioObserver>())
		{
			Observer->RegisterStateRequest(this, AudioStateTag, bRequestActive);
		}
	}
}
