#include "Flow/Nodes/FlowNode_AddAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "FlowSubsystem.h"
#include "Character/StillHearMainCharacter.h"
#include "Engine/World.h"
#include "SaveSystem/SaveSubsystem.h"

UFlowNode_AddAbility::UFlowNode_AddAbility()
{
#if WITH_EDITOR
	Category = TEXT("Abilities");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif

	InputPins.Empty();
	InputPins.Add(FFlowPin(FName("In")));

	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FName("Out")));
}

void UFlowNode_AddAbility::ExecuteInput(const FName& PinName)
{
	if (PinName == FName("In"))
	{
		UWorld* World = GetWorld();
		if (!World) return;

		TArray<AActor*> TargetActors;

		// Resolve Target Actors
		if (TargetIdentityTag.IsValid())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UFlowSubsystem>())
				{
					const TSet<AActor*>& FoundActors = FlowSubsystem->GetFlowActorsByTag(TargetIdentityTag, AActor::StaticClass());
					TargetActors = FoundActors.Array();
				}
			}
		}
		else
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (AActor* PlayerPawn = PC->GetPawn())
				{
					TargetActors.Add(PlayerPawn);
				}
			}
		}

		for (AActor* Actor : TargetActors)
		{
			if (AStillHearMainCharacter* MainChar = Cast<AStillHearMainCharacter>(Actor))
			{
				if (UAbilitySystemComponent* ASC = MainChar->GetAbilitySystemComponent())
				{
					TSubclassOf<UGameplayAbility> AbilityClass = MainChar->GetAbilityClassByType(AbilityToGrant);
					if (AbilityClass)
					{
						// Check if the ability is already granted to avoid duplicates
						if (!ASC->FindAbilitySpecFromClass(AbilityClass))
						{
							ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1, MainChar));
						}
					}
				}
			}
		}

		// Record permanent unlock if requested
		if (bPermanentUnlock)
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USaveSubsystem* SaveSS = GI->GetSubsystem<USaveSubsystem>())
				{
					SaveSS->UnlockPermanentAbility(AbilityToGrant);
				}
			}
		}


		TriggerOutput(FName("Out"), true);
	}
}
