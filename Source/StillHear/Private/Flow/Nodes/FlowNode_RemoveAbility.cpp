#include "Flow/Nodes/FlowNode_RemoveAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "FlowSubsystem.h"
#include "Character/StillHearMainCharacter.h"
#include "Engine/World.h"
#include "SaveSystem/SaveSubsystem.h"

UFlowNode_RemoveAbility::UFlowNode_RemoveAbility()
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

void UFlowNode_RemoveAbility::ExecuteInput(const FName& PinName)
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
					TSubclassOf<UGameplayAbility> AbilityClass = MainChar->GetAbilityClassByType(AbilityToRemove);
					if (AbilityClass)
					{
						FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
						if (Spec)
						{
							ASC->ClearAbility(Spec->Handle);
						}
					}
				}
			}
		}

		// Update permanent save data
		if (bPermanentRemove)
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USaveSubsystem* SaveSS = GI->GetSubsystem<USaveSubsystem>())
				{
					SaveSS->RemovePermanentAbility(AbilityToRemove);
				}
			}
		}

		TriggerOutput(FName("Out"), true);
	}
}
