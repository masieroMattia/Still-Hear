#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_EnableHitBox.generated.h"

/**
 * AnimNotify that changes the collision state of a UShapeComponent on the owner
 * Used for single-point activation or deactivation
 */
UCLASS()
class STILLHEAR_API UAN_EnableHitBox : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_EnableHitBox();

#pragma region UPROPERTIES
public:
	// ComponentTag used to find the hit box on the owning actor
	UPROPERTY(EditAnywhere, Category = "HitBox")
	FGameplayTag HitBoxTag;

	// Whether to enable or disable collision
	UPROPERTY(EditAnywhere, Category = "HitBox")
	bool bEnableCollision;

	// If true, will send a Gameplay Event to the owner's ASC
	UPROPERTY(EditAnywhere, Category = "GAS")
	bool bSendGameplayEvent = false;

	// The tag of the Gameplay Event to send
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (EditCondition = "bSendGameplayEvent"))
	FGameplayTag EventTag;
#pragma endregion

#pragma region METHODS
protected:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	static UShapeComponent* FindHitBoxComponent(const AActor* Owner, const FName& Tag);
#pragma endregion
};
