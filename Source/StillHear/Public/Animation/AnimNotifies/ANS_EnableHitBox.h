#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_EnableHitBox.generated.h"

/**
 * Generic AnimNotifyState that enables a UShapeComponent on the owner for the duration of the notify window
 * Finds the component by its ComponentTag
 */
UCLASS()
class STILLHEAR_API UANS_EnableHitBox : public UAnimNotifyState
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	// ComponentTag used to find the hit box on the owning actor
	UPROPERTY(EditAnywhere, Category = "HitBox")
	FGameplayTag HitBoxTag;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual FString GetNotifyName_Implementation() const override;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	static UShapeComponent* FindHitBoxComponent(const AActor* Owner, const FName& Tag);
#pragma endregion
};


