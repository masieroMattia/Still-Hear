#pragma once

#include "CoreMinimal.h"
#include "TargetableDrivableObj.h"
#include "OverlapHitObj.generated.h"

class UGameplayEffect;

UCLASS()
class STILLHEAR_API AOverlapHitObj : public ATargetableDrivableObj
{
	GENERATED_BODY()

public:
	AOverlapHitObj();

#pragma region UPROPERTIES
protected:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> ProximitySphereComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable Settings|Effect")
	TSubclassOf<UGameplayEffect> HitEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Overlap Config")
	float OverlapCheckInterval = 0.2f; // How often (in seconds) to check for overlapping characters
#pragma endregion

#pragma region VARIABLES
private:
	FTimerHandle OverlapCheckTimerHandle;
#pragma endregion
	
#pragma region OVERRIDE METHODS
protected:
	virtual void BeginPlay() override;
	virtual void HandleTimelineFinished() override;
	virtual void HitTarget(int32 DeflectionsCount = 0) override;
	virtual void Reset() override;
#pragma endregion

#pragma region METHODS
private:
	void StartTimerForOverlapCheck();
	void CheckForOverlappingCharacter();

	UFUNCTION()
	void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion
};
