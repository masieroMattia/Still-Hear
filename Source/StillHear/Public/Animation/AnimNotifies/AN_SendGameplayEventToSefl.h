#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SendGameplayEventToSefl.generated.h"

UCLASS()
class STILLHEAR_API UAN_SendGameplayEventToSelf : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
public:
	UPROPERTY(EditAnywhere, Category = "GameplayEvent")
	FGameplayTag EventTag;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion
};
