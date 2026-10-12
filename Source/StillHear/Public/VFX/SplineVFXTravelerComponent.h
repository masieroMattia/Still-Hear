#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SplineVFXTravelerComponent.generated.h"

class UNiagaraComponent;
class AInteractableObj;
class USplineComponent;
class UAudioComponent;
class UNiagaraSystem;
class USoundBase;

/**
 * Represents a single VFX instance currently traveling along the spline
 * Multiple instances can be active at the same time
 * Each one tracks its own elapsed time, total duration, and travel direction
 */
USTRUCT(BlueprintType)
struct FSplineTravelerInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComponent;

	float ElapsedTime = 0.0f;
	float InstanceDuration = 0.0f;
	bool bIsReversed = false;

	// Default constructor for TArray
	FSplineTravelerInstance() {}
	// Convenience constructor to initialize all properties at once
	FSplineTravelerInstance(UNiagaraComponent* InVFX, const float InDuration, const bool bInReverse) : VFXComponent(InVFX), InstanceDuration(InDuration), bIsReversed(bInReverse) {}
};

/**
 * Takes the spline of the owner actor and spawns VFX/Sounds that travel along it
 * Multiple VFX can travel simultaneously, but only one looping sound is allowed
 * Supports triggering linked interactables upon completion
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API USplineVFXTravelerComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region CONSTRUCTOR
public:
	USplineVFXTravelerComponent();
#pragma endregion

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TravelVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> StartVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> EndVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> TravelLoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> StartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> EndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TObjectPtr<UCurveFloat> TravelCurve;

	// Fallback speed if no curve is used (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FallbackTravelSpeed = 500.0f;

	// Actors to trigger when a forward traveler reaches the end of the spline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<TObjectPtr<AInteractableObj>> ForwardLinkedObjects;

	// Actors to trigger when a reverse traveler reaches the start of the spline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<TObjectPtr<AInteractableObj>> ReverseLinkedObjects;
#pragma endregion

#pragma region VARIABLES
private:
	UPROPERTY()
	TArray<FSplineTravelerInstance> ActiveTravelers;
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopAudioComponent;
	UPROPERTY()
	TObjectPtr<USplineComponent> CachedSpline;

	// Ensures we have a valid spline component from the owner
	void FindSpline();
	
	// Updates the state of the looping sound based on active travelers
	void UpdateLoopSoundState();
#pragma endregion
	
#pragma region METHODS
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#pragma endregion
	
#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Spline VFX")
	void TriggerTravel(bool bReverse = false);
#pragma endregion

};
