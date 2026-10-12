#pragma once

#include "CoreMinimal.h"
#include "DrivableObj.h"
#include "Interfaces/Targetable.h"
#include "VFX/TargetMarkerNiagaraComponent.h"
#include "TargetableDrivableObj.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetHit, int32, DeflectionsCount);

UCLASS()
class STILLHEAR_API ATargetableDrivableObj : public ADrivableObj, public ITargetable
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bIsTargetable", hideeditcondition = "!bIsTargetable"), Category = "Components")
	TObjectPtr<UTargetMarkerNiagaraComponent> TargetNiagaraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bIsTargetable", hideeditcondition = "!bIsTargetable"), Category = "Components")
	TObjectPtr<UNiagaraComponent> HitNiagaraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bIsTargetable = true;
	
	// If true, the object becomes untargetable after being hit once
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bTargetOnce = false;

	// If true, the object will perform its movement/animation when hit. Default is false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bMoveOnHit = false;

	// If true and bMoveOnHit is false, the object will automatically reset its interaction state after AutoResetDelay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Settings|Config")
	bool bAutoReset = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bAutoReset"), Category = "Interactable Settings|Config")
	float AutoResetDelay = 1.0f;
#pragma endregion
	
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "Interactable Settings|Events")
	FOnTargetHit OnTargetHit;
#pragma endregion
	
#pragma region VARIABLES
protected:
	bool bCachedIsTargetable = false;
	bool bIsExecutingHit = false;
	bool bWasHitInteraction = false;
	
	// ── Checkpoint Snapshot ──
	UPROPERTY(SaveGame)
	bool bHasTargetableCheckpointSnapshot = false;
	UPROPERTY(SaveGame)
	bool bCheckpointIsTargetable = false;
	UPROPERTY(SaveGame)
	bool bCheckpointWasHitInteraction = false;
#pragma endregion
	
#pragma region CONSTRUCTOR
public:
	ATargetableDrivableObj();
#pragma endregion
	
#pragma region UFUNCTIONS
	UFUNCTION(BlueprintPure, Category = "Interactable Settings")
	bool IsHitInteraction() const { return bWasHitInteraction; }
#pragma endregion
	
#pragma region OVERRIDE METHODS
	virtual void BeginPlay() override;
	virtual void Reset() override;
	virtual void ExecuteStartInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void EndInteraction(TObjectPtr<ACharacter> Interactor = nullptr) override;
	virtual void SaveCheckpointState() override;
	virtual void ClearCheckpointState() override;
#pragma endregion
	
#pragma region INTERFACE METHODS
	virtual void SetTargeted() override;
	virtual void SetUntargeted() override;
	
	virtual void HitTarget(int32 DeflectionsCount = 0) override;
	virtual void StopHitTarget() override;
	virtual bool IsTargetable() const override;

protected:
	virtual bool ShouldActivateOnHit(int32 DeflectionsCount) const { return true; }
#pragma endregion
};
