#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemiesAI/Pawns/Base/StillHearAICharacterBase.h"
#include "StillHearAIControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusTagChanged, FGameplayTag, NewStatusTag);

UCLASS(Abstract)
class STILLHEAR_API AStillHearAIControllerBase : public AAIController
{
	GENERATED_BODY()

#pragma region UPROPERTY
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Behaviour Tree")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AStillHearAICharacterBase> NPCRef;
#pragma endregion 

#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "AI Status|Events")
	FOnStatusTagChanged OnStatusTagChanged;
#pragma endregion
	
#pragma region VARIABLES	
protected:
	FGameplayTag CurrentStatusTag;
	
	bool PossessHappened = false;
	bool BeginPlayHappened = false;
#pragma endregion 
	
#pragma region CONSTRUCTOR
public:
	explicit AStillHearAIControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
#pragma endregion 

#pragma region UFUNCTION
public:
	UFUNCTION(BlueprintPure, Category = "AI Status")
	FGameplayTag GetCurrentStatusTag() const { return CurrentStatusTag; }
	
	UFUNCTION(BlueprintCallable, Category = "AI Status")
	void UpdateCurrentStatusTag(E_AITag NewTag);

	UFUNCTION(BlueprintPure, Category = "AI Status")
	bool CheckCurrentStatusTag(E_AITag TagToCheck) const;

protected:
	/*
	 * IMPORTANT: Remember to override this function in any child class to define the behaviour of the AI
	 * at the moment of receiving an update from the perception component!
	 */
	UFUNCTION()
	virtual void PerceptionEventReceived(AActor* UpdatedActor, FAIStimulus Stimulus) {};
#pragma endregion 
	
#pragma region METHODS
public:
	virtual void Tick(float DeltaTime) override;

	/*
	 * TEAM ID
	 */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	AStillHearAICharacterBase* GetNPCRef() const { return NPCRef; };
	
	virtual void ResetAIState();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;


	/*
	 * These functions will read from the Data Asset and override the sense in the perception component, so
	 * the designers will only have to touch the data asset and never touch the AIController.
	 * In any case, each child of this class will decide either to implement these functions or not depending
	 * on if they have the corresponding sense or not.
	 */
	virtual void SetupSightInfo() {};
	virtual void SetupHearingInfo() {};
	
	virtual void SetupBlackboardKeys();
#pragma endregion 
};
