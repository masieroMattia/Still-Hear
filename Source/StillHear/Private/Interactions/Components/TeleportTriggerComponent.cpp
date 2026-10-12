#include "Interactions/Components/TeleportTriggerComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "CommonActivatableWidget.h"
#include "UI/Subsystem/UISubsystem.h"
#include "Character/StillHearMainCharacter.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameFramework/PlayerController.h"

UTeleportTriggerComponent::UTeleportTriggerComponent()
{
	// Enable overlap events and restrict to Player collision channel by default
	SetGenerateOverlapEvents(true);
	AllowedCollisionChannels.Add(ECustomCollision::Player);
}

void UTeleportTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	AStillHearMainCharacter* Player = Cast<AStillHearMainCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	FVector DestLocation = FVector::ZeroVector;
	FRotator DestRotation = FRotator::ZeroRotator;

	if (const AActor* Target = DestinationPoint.Get())
	{
		DestLocation = Target->GetActorLocation();
		DestRotation = Target->GetActorRotation();
	}
	else if (!DestinationPoint.IsNull())
	{
		// Load soft reference synchronously to resolve destination
		if (const AActor* LoadedTarget = DestinationPoint.LoadSynchronous())
		{
			DestLocation = LoadedTarget->GetActorLocation();
			DestRotation = LoadedTarget->GetActorRotation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UTeleportTriggerComponent: DestinationPoint is not set on %s!"), *GetName());
		return;
	}

	// Show widget if enabled
	if (bShowWidget && WidgetClass && WidgetLayerTag.IsValid())
	{
		HideWidget(); // Dismiss any prior widget instance

		const UWorld* World = GetWorld();
		if (World)
		{
			if (const APlayerController* PC = Cast<APlayerController>(Player->GetController()))
			{
				if (const ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>())
					{
						ActiveWidget = UISubsystem->PushWidgetToLayer(WidgetLayerTag, WidgetClass, false);
					}
				}
			}

			if (ActiveWidget)
			{
				World->GetTimerManager().SetTimer(WidgetTimerHandle, this, &UTeleportTriggerComponent::HideWidget, WidgetDisplayDuration, false);
			}
		}
	}

	// Increment base trigger counter
	IncrementTriggerCount();

	if (TeleportDelay > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TeleportTimerHandle);

			FTimerDelegate TeleportDelegate;
			TeleportDelegate.BindUObject(this, &UTeleportTriggerComponent::ExecuteTeleport, Player, DestLocation, DestRotation);
			World->GetTimerManager().SetTimer(TeleportTimerHandle, TeleportDelegate, TeleportDelay, false);
		}
	}
	else
	{
		ExecuteTeleport(Player, DestLocation, DestRotation);
	}
}

void UTeleportTriggerComponent::HideWidget()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WidgetTimerHandle);
	}

	if (ActiveWidget)
	{
		ActiveWidget->DeactivateWidget();
		ActiveWidget = nullptr;

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			PC->SetInputMode(FInputModeGameOnly());
	}
}

void UTeleportTriggerComponent::Reset()
{
	HideWidget();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TeleportTimerHandle);
	}
	Super::Reset();
}

void UTeleportTriggerComponent::ExecuteTeleport(AStillHearMainCharacter* Player, FVector DestLocation, FRotator DestRotation)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TeleportTimerHandle);
	}

	if (!Player || !IsValid(Player))
	{
		return;
	}

	// Teleport Character
	Player->SetActorLocationAndRotation(DestLocation, DestRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Update PlayerController control rotation to match destination orientation
	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		PC->SetControlRotation(DestRotation);
	}
}

