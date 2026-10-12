#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "AN_ReportNoiseEvent.generated.h"

UCLASS()
class STILLHEAR_API UAN_ReportNoiseEvent : public UAnimNotify
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "AI Sense")
	float Loudness = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Sense")
	float NoiseRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Sense")
	FName Tag = NAME_None;
	
#pragma endregion
	
#pragma region METHODS
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion
};
