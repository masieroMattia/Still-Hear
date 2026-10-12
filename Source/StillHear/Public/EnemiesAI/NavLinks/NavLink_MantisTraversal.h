#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLink_MantisTraversal.generated.h"

/**
 * Nav Link Proxy that allows AI characters to perform a jump traversal when they reach it.
 * It calculates the necessary jump velocity based on the distance and height difference between the start and end points, as well as any obstacles in the way 
 * The jump is only executed if the required peak height is within the specified MaxJumpHeight
 */
UCLASS()
class STILLHEAR_API ANavLink_MantisTraversal : public ANavLinkProxy
{
	GENERATED_BODY()

#pragma region UPROPERTIES
protected:
    /** 
     * How long (seconds) the NavLink stays disabled after a successful or failed attempt
	 * During this time the navigation system reroutes agents away from this link 
	 */
    UPROPERTY(EditAnywhere, Category = "AI Traversal")
    float LinkDisableDuration = 3.0f;
#pragma endregion

#pragma region VARIABLES
private:
	FTimerHandle LinkReenableTimerHandle;
#pragma endregion 

#pragma region CONSTRUCTOR
public:
	ANavLink_MantisTraversal();
#pragma endregion 

#pragma region UFUNCTION
private:
	UFUNCTION()
	void NotifySmartLinkReached(AActor* MovingActor, const FVector& DestinationPoint);
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;

private:
	// Disables this NavLink for Duration seconds, then re-enables it
	void DisableLinkTemporarily(float Duration);
#pragma endregion
};
