#include "EnemiesAI/Pawns/Mantis/AIMantisCharacter.h"

#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "UI/AI/EnemyStatus.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Perception/AISense_Touch.h"
#include "Components/CapsuleComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemiesAI/Controllers/Mantis/AIMantisController.h"
#include "EnemiesAI/Utility/Components/PerceptionVisualizerComponent.h"

#pragma region CONSTRUCTOR
AAIMantisCharacter::AAIMantisCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIType = E_AIType::MANTIS;
	
	PerceptionVisualizerComp = CreateDefaultSubobject<UPerceptionVisualizerComponent>("Perception Visualizer Component");

	// Attack hit box attached to the hand socket, disabled by default
	AttackHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitBox"));
	AttackHitBox->SetupAttachment(GetMesh(), FName("hand_r_socket"));
	AttackHitBox->ComponentTags.Add(TAG_AttackHitBox.GetTag().GetTagName());
	AttackHitBox->SetGenerateOverlapEvents(true);
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitBox->SetCollisionObjectType(ECC_WorldDynamic);
	AttackHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitBox->SetCollisionResponseToChannel(ECustomCollision::Player, ECR_Overlap);
	AttackHitBox->SetBoxExtent(FVector(30.0f, 30.0f, 30.0f));

	AttackFeedbackVfxLocation = CreateDefaultSubobject<USceneComponent>("Attack Feedback VFX Location");
	AttackFeedbackVfxLocation->SetupAttachment(GetMesh());
	
	GetCapsuleComponent()->SetCollisionObjectType(ECustomCollision::Mantis);
}
#pragma endregion
	
#pragma region METHODS
void AAIMantisCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	bIsDormant = bStartDormant;
}

void AAIMantisCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (bStartDormant)
	{
		bIsDormant = false;
		EnterDormancy();
	}

	// Start the periodic NavMesh consistency check (Mantis-only)
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			NavMeshCheckTimerHandle,
			this,
			&AAIMantisCharacter::CheckNavMeshConsistency,
			NavMeshCheckInterval,
			true
		);
	}
}

void AAIMantisCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear the NavMesh check timer before the base EndPlay runs
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NavMeshCheckTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AAIMantisCharacter::CheckNavMeshConsistency()
{
	if (!GetWorld())
		return;

	// Never teleport while the Mantis is airborne — would cause the ugly mid-air snap
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
		return;

	// Also skip while the NavLink jump is in progress
	if (bIsPerformingNavLinkJump)
		return;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
		return;

	const FVector CurrentLoc = GetActorLocation();
	FNavLocation NavLoc;

	const FVector Tolerance(100.f, 100.f, 250.f);
	const bool bIsOnNavMesh = NavSys->ProjectPointToNavigation(CurrentLoc, NavLoc, Tolerance);

	if (bIsOnNavMesh)
	{
		// Back on the mesh: forget any prior isolated false positives
		ConsecutiveNavMeshCheckFailures = 0;
		return;
	}

	// Require several consecutive failures before snapping
	++ConsecutiveNavMeshCheckFailures;
	if (ConsecutiveNavMeshCheckFailures < NavMeshCheckFailureThreshold)
		return;

	ConsecutiveNavMeshCheckFailures = 0;

	const FVector SearchExtent(NavMeshSearchRadius, NavMeshSearchRadius, 1000.f);
	if (NavSys->ProjectPointToNavigation(CurrentLoc, NavLoc, SearchExtent))
	{
		FVector TargetLoc = NavLoc.Location;

		if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			TargetLoc.Z += Capsule->GetScaledCapsuleHalfHeight() + 2.0f;
		}

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
		}

		SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AAIMantisCharacter::SetIsPerformingNavLinkJump(const bool bJumping)
{
	bIsPerformingNavLinkJump = bJumping;
}

void AAIMantisCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Clear the jump flag so CheckNavMeshConsistency and the NavLink guard work normally again
	bIsPerformingNavLinkJump = false;
}

void AAIMantisCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, const bool bSelfMoved, const FVector HitLocation, const FVector HitNormal, const FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (Other && OtherComp && GetMantisDataAsset())
	{
		// Check if the hit component belongs to the specified touch channel
		if (OtherComp->GetCollisionObjectType() == GetMantisDataAsset()->TouchReportChannel)
			UAISense_Touch::ReportTouchEvent(GetWorld(), this, Other, HitLocation);
	}
}

void AAIMantisCharacter::Activate()
{
	if (!bIsDormant)
		return;

	bIsDormant = false;

	// Re-enable tick immediately so animation can play and update
	SetActorTickEnabled(true);

	bool bMontageStarted = false;
	if (WakeUpMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			const float Duration = PlayAnimMontage(WakeUpMontage);
			if (Duration > 0.0f)
			{
				bMontageStarted = true;
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &AAIMantisCharacter::OnWakeUpMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, WakeUpMontage);
			}
		}
	}

	if (!bMontageStarted)
	{
		FinishActivation();
	}
}

void AAIMantisCharacter::OnWakeUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == WakeUpMontage)
	{
		if (!bIsDormant)
		{
			FinishActivation();
		}
	}
}

void AAIMantisCharacter::FinishActivation()
{
	// Re-enable movement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		MoveComp->SetMovementMode(MOVE_Walking);

	// Restart Behavior Tree and perception
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIC->GetBrainComponent())
			BrainComp->RestartLogic();

		if (UAIPerceptionComponent* PerceptionComp = AIC->GetPerceptionComponent())
			PerceptionComp->Activate();
	}
}

void AAIMantisCharacter::EnterDormancy()
{
	if (bIsDormant)
		return;

	bIsDormant = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIC->GetBrainComponent())
			BrainComp->StopLogic(TEXT("Dormant"));

		if (UAIPerceptionComponent* PerceptionComp = AIC->GetPerceptionComponent())
			PerceptionComp->Deactivate();
	}
	
	SetActorTickEnabled(false);
}

void AAIMantisCharacter::ResetAfterDeath()
{
	Super::ResetAfterDeath();

	// Clear the NavLink jump flag (Mantis-specific)
	bIsPerformingNavLinkJump = false;

	// If this mantis was placed as dormant, go back to dormant
	if (bStartDormant)
	{
		EnterDormancy();
	}
}

void AAIMantisCharacter::NotifyHitByProjectile(AActor* HitInstigator)
{
	if (bIsDormant || !IsValid(HitInstigator))
		return;

	if (AAIMantisController* MantisController = Cast<AAIMantisController>(GetController()))
		MantisController->ForceHuntTarget(HitInstigator);
}

void AAIMantisCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = GetActorLocation();
	if (const UAIMantisInfo_DataAsset* DataAsset = GetMantisDataAsset())
	{
		OutLocation.Z += DataAsset->SightHeight;
	}
	else
	{
		OutLocation.Z += BaseEyeHeight;
	}
	OutRotation = GetViewRotation();
}
#pragma endregion