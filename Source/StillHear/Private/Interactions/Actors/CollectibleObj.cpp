#include "Interactions/Actors/CollectibleObj.h"

#include "Engine/LocalPlayer.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "AbilitySystemComponent.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/Character.h"
#include "SaveSystem/SaveSubsystem.h"
#include "UI/Subsystem/UISubsystem.h"
#include "UI/Widgets/ImagePopupWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Data/DataTables/CollectibleData.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

#pragma region CONSTRUCTOR
ACollectibleObj::ACollectibleObj()
{
	PrimaryActorTick.bCanEverTick = false;

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(FName("RotatingMovementComponent"));
}
#pragma endregion

#pragma region METHODS
void ACollectibleObj::BeginPlay()
{
	Super::BeginPlay();
}

void ACollectibleObj::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULevelSequencePlayer* RawPlayer = CachedSequencePlayer.Get())
	{
		// Safely unbind our delegate to prevent memory leaks in PIE
		RawPlayer->OnFinished.RemoveDynamic(this, &ACollectibleObj::OnCinematicFinished);
		RawPlayer->Stop();
	}

	CachedSequencePlayer = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ACollectibleObj::TriggerCinematic()
{
	if (!CollectibleCinematicActor)
	{
		OnContentFinished();
		return;
	}

	ULevelSequencePlayer* Player = CollectibleCinematicActor->GetSequencePlayer();

	if (Player)
	{
		CachedSequencePlayer = Player;
		Player->OnFinished.RemoveDynamic(this, &ACollectibleObj::OnCinematicFinished); // Ensure no duplicate bindings
		Player->OnFinished.AddDynamic(this, &ACollectibleObj::OnCinematicFinished);

		if (CachedInteractor)
		{
			CollectibleCinematicActor->SetBindingByTag(TAG_MainCharacter.GetTag().GetTagName(), TArray<AActor*>{ CachedInteractor.Get() });
		}

		Player->Play();
	}
	else
	{
		OnContentFinished();
	}
}

void ACollectibleObj::TriggerUIWidget()
{
	// If the player died while this interaction was in progress, don't pop up a pausing widget
	if (CachedInteractor)
	{
		if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedInteractor.Get()))
		{
			if (ASC->GetTagCount(TAG_Status_Death) > 0)
			{
				OnContentFinished();
				return;
			}
		}
	}

	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	const ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;
	UUISubsystem* UISubsystem = LP ? LP->GetSubsystem<UUISubsystem>() : nullptr;

	if (UISubsystem && CollectibleWidgetClass && CollectibleWidgetLayerTag.IsValid())
	{
		if (UCommonActivatableWidget* SpawnedWidget = UISubsystem->PushWidgetToLayer(CollectibleWidgetLayerTag, CollectibleWidgetClass, false, true))
		{
			if (UImagePopupWidget* CollWidget = Cast<UImagePopupWidget>(SpawnedWidget))
			{
				// GetRow safely returns nullptr if the RowHandle is null or invalid
				if (const FCollectibleData* RowData = CollectibleDataRow.GetRow<FCollectibleData>(TEXT("CollectibleContext")))
				{
					CollWidget->InitializeImagePopup(RowData->CollectibleText, 0.0f, RowData->CollectibleImage, RowData->CollectibleMaterial);
				}
			}
		}
	}

	OnContentFinished();
}

void ACollectibleObj::OnContentFinished()
{
	EndInteraction(CachedInteractor);
}
#pragma endregion

#pragma region UFUNCTIONS
void ACollectibleObj::OnCinematicFinished()
{
	if (ULevelSequencePlayer* RawPlayer = CachedSequencePlayer.Get())
	{
		RawPlayer->OnFinished.RemoveDynamic(this, &ACollectibleObj::OnCinematicFinished);
	}
	CachedSequencePlayer = nullptr;

	// Show the widget first, then OnContentFinished is called by TriggerUIWidget
	if (CollectibleBehavior == ECollectibleBehavior::PlayCinematicThenUI)
		TriggerUIWidget();
	else
		OnContentFinished();
}
#pragma endregion

#pragma region INTERFACE METHODS
void ACollectibleObj::ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor)
{
	// Cache the interactor so we can signal the GA when we're done
	CachedInteractor = Interactor;

	Super::ExecuteStartInteraction(Interactor);

	switch (CollectibleBehavior)
	{
	case ECollectibleBehavior::ShowUIWidget:
		TriggerUIWidget();
		break;
	case ECollectibleBehavior::PlayCinematic:
	case ECollectibleBehavior::PlayCinematicThenUI:
		TriggerCinematic();
		break;
	}
}

void ACollectibleObj::EndInteraction(const TObjectPtr<ACharacter> Interactor)
{
	// Guard: only run if an actual interaction was in progress
	if (!bIsInteracting)
		return;

	Super::EndInteraction(Interactor);

	bIsCollected = true;
	UE_LOG(LogTemp, Log, TEXT("[CollectibleObj] EndInteraction: Collected %s, requesting save"), *GetName());

	// Save game status automatically
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USaveSubsystem* SaveSubsystem = GI->GetSubsystem<USaveSubsystem>())
			{
				// Register globally (persists across slot deletes)
				const FName RowName = CollectibleDataRow.RowName;
				SaveSubsystem->CollectCollectible(RowName);

				const int32 Slot = SaveSubsystem->GetCurrentSlotSave();
				if (Slot > 0)
				{
					SaveSubsystem->RequestSaveSlotAsync(Slot);
				}
			}
		}
	}

	// Tell the interaction GA to end cleanly so the interaction tag is removed
	ACharacter* Target = Interactor ? Interactor.Get() : CachedInteractor.Get();
	if (Target)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Cast<AActor>(Target)))
		{
			const FGameplayEventData EventData;
			ASC->HandleGameplayEvent(TAG_Event_StopInteraction, &EventData);
		}
	}

	// Hide the actor and disable all interactions
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ACollectibleObj::OnPreSave_Implementation()
{
}

void ACollectibleObj::OnPostLoad_Implementation()
{
	if (bIsCollected)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);

		if (SphereComponent)
			SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		if (StaticMeshComponent)
			StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

		// Activate associated actors (such as Niagara VFX actors) that should remain active
		for (AActor* ActorToActivate : ActorsToActivateOnLoad)
		{
			if (IsValid(ActorToActivate))
			{
				ActorToActivate->SetActorHiddenInGame(false);

				TArray<UNiagaraComponent*> NiagaraComponents;
				ActorToActivate->GetComponents<UNiagaraComponent>(NiagaraComponents);
				for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
				{
					if (NiagaraComp)
					{
						NiagaraComp->Activate(true);
					}
				}
			}
		}

		// Notify blueprints that this collectible has loaded in its collected state
		OnCollectibleLoadedCollected();
	}
}
#pragma endregion