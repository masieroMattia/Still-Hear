#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InteractionBaseData.generated.h"

class UGameplayEffect;
class UForceFeedbackEffect;

UCLASS(NotBlueprintable)
class STILLHEAR_API UInteractionBaseData : public UDataAsset
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// Radius within which the interaction can be triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Config")
	float InteractRadius = 200.0f;
	// Distance from the interactable at which the character should move to before starting the interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Config")
	float MoveToAcceptanceRadius = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Config")
	float MaxZDifferenceToInteract = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction Settings|Config")
	float SpotAcceptanceRadius = 5.0f;
	// Gameplay tags to identify the type of interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Config")
	FGameplayTagContainer InteractionTags = FGameplayTagContainer();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Effects")
	TSubclassOf<UGameplayEffect> MoveToEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings|Feedback")
	TObjectPtr<UForceFeedbackEffect> InteractionForceFeedback;
#pragma endregion
};