#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AudioStateComponent.generated.h"

/**
 * Universal component that can be added to any actor to request an audio state.
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API UAudioStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAudioStateComponent();

	/** The audio state tag this component requests when active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	FGameplayTag AudioStateTag;

	/** If true, this component will automatically tick and detect overlap with the player pawn to request/release the state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bAutoDetectOverlap = true;

	/** If true, this component starts active or is currently requested */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bRequestActive = false;

	/** Activate or deactivate this audio state request */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAudioStateActive(bool bNewActive);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void NotifyObserver();
};
