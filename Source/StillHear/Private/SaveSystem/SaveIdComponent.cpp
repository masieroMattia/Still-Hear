#include "SaveSystem/SaveIdComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SaveSystem/SaveSubsystem.h"
#include "kismet/KismetGuidLibrary.h"

USaveIdComponent::USaveIdComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FGuid USaveIdComponent::GetSaveId() const
{
	const AActor* Owner = GetOwner();
	if (!Owner || Owner->Tags.Contains(FName("DynamicallySpawned")))
	{
		return FGuid();
	}

	const ULevel* Level = Owner->GetLevel();
	const UWorld* World = Owner->GetWorld();
	if (!Level || !World)
	{
		return FGuid();
	}

	const FName StableLevelKey = USaveSubsystem::GetStableLevelKey(World, Level);
	const FString StablePath = StableLevelKey.ToString() + TEXT(".") + Owner->GetName();
	return FGuid::NewDeterministicGuid(StablePath);
}


void USaveIdComponent::GenerateID()
{
	EnsureId(false);
}

void USaveIdComponent::RegenerateID()
{
	EnsureId(true);
}

void USaveIdComponent::EnsureId(const bool bForceNew)
{
	if (!GetOwner()) 
		return;
	
	if (bForceNew || !SaveId.IsValid())
	{
#if WITH_EDITOR
		if (const UWorld* World = GetWorld(); World && !World->IsGameWorld())
		{
			if (AActor* Owner = GetOwner())
			{
				Owner->Modify(true);
			}
			Modify(true);
		}
#endif

		SaveId = UKismetGuidLibrary::NewGuid();
   	    bNull = true;
	} 
}

#if WITH_EDITOR
void USaveIdComponent::PostEditImport()
{
	Super::PostEditImport();
	if (!bNull)
	{
		EnsureId(false);
	}
	else 
	{
		EnsureId(true);
	}
}
#endif

void USaveIdComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
#if WITH_EDITOR
	if (const UWorld* World = GetWorld(); World && !World->IsGameWorld())
	{
		EnsureId(false);
	}
#endif
}

void USaveIdComponent::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	if (const UWorld* World = GetWorld(); World && !World->IsGameWorld())
	{
		EnsureId(false);
	}
#endif
}
