#include "GameplayAbilitySystem/Tasks/AT_NavigateTo.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region CONSTRUCTOR
UAT_NavigateTo::UAT_NavigateTo()
{
	bTickingTask = true;

}
#pragma endregion
	
#pragma region UFUNCTIONS
UAT_NavigateTo* UAT_NavigateTo::NavigateTo(UGameplayAbility* OwningAbility, const FVector& InTargetLocation, const float InAcceptanceRadius, const FVector& InFacingDirection)
{
	UAT_NavigateTo* NavigateToTask = NewAbilityTask<UAT_NavigateTo>(OwningAbility);
	NavigateToTask->TargetLocation = InTargetLocation;
	NavigateToTask->AcceptanceRadius = InAcceptanceRadius;
	NavigateToTask->FacingDirection = InFacingDirection;
	
	return NavigateToTask;
}
#pragma endregion
	
#pragma region METHODS
void UAT_NavigateTo::Activate()
{
	Super::Activate();

	OriginalTargetLocation = TargetLocation;

	const ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
	{
		EndTask(); 
		return;
	}

	bHasReachedTarget = false;
}

void UAT_NavigateTo::TickTask(const float DeltaTime)
{
	Super::TickTask(DeltaTime);

   ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
   if (!Character)
   {
      EndTask();
      return;
   }
   
   if (bHasReachedTarget)
   {
      FVector DirectionToFace = FacingDirection.IsNearlyZero() ? (OriginalTargetLocation - Character->GetActorLocation()) : FacingDirection;
      DirectionToFace.Z = 0.0f; // Ignore vertical difference for rotation

      if (DirectionToFace.IsNearlyZero())
      {
         OnTargetLocationReached.Broadcast();
         EndTask();
         return;
      }
      
      const FRotator TargetRotation = DirectionToFace.Rotation();
      const float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(Character->GetActorRotation().Yaw, TargetRotation.Yaw));
      
      if (AngleDifference <= 5.0f) // Consider the target reached if the character is facing within 5 degrees of the target direction
      {
         OnTargetLocationReached.Broadcast();
         EndTask();
         return;
      }
      
      const FRotator NewRotation = FMath::RInterpTo(Character->GetActorRotation(), TargetRotation, DeltaTime, Character->GetCharacterMovement()->RotationRate.Yaw);
      Character->SetActorRotation(NewRotation);
   }
   else
   {
      TimeSinceActivation += DeltaTime;
      
      const FVector CurrentLocation = Character->GetActorLocation();
      const float SquareDistanceToTarget = FVector::DistSquared(CurrentLocation, TargetLocation);
      
      if (SquareDistanceToTarget <= FMath::Square(AcceptanceRadius))
      {
         bHasReachedTarget = true;
         return;
      }

      // Calculate horizontal direction to the target
      FVector Direction = TargetLocation - CurrentLocation;
      Direction.Z = 0.0f;
      
      const float DistanceToTarget = Direction.Size();
      Direction.Normalize();
      
      // Scale movement input based on distance for a smooth arrival (slows down within 100 units)
      const float MovementScale = FMath::Clamp(DistanceToTarget / 100.0f, 0.2f, 1.0f);
      
      // Move the character using standard input with the calculated scale
      Character->AddMovementInput(Direction, MovementScale);
      
      // Track if the character has started moving before checking if it stopped
      if (!bHasStartedMoving && Character->GetVelocity().SizeSquared() > FMath::Square(10.0f))
         bHasStartedMoving = true;
      
      const bool bStoppedMoving = bHasStartedMoving && Character->GetVelocity().SizeSquared() < FMath::Square(5.0f);
      const bool bFailedToMove = !bHasStartedMoving && TimeSinceActivation > 0.15f; 
      
      if (bStoppedMoving || bFailedToMove)
         bHasReachedTarget = true;
   }
}
#pragma endregion
