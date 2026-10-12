#pragma once

#include "CoreMinimal.h"
#include "AN_ReportNoiseEvent.h"
#include "AN_ReportFoostepNoiseEvent.generated.h"

UCLASS()
class STILLHEAR_API UAN_ReportFoostepNoiseEvent : public UAN_ReportNoiseEvent
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float MinSpeedThreshold = 55.0f;
};
