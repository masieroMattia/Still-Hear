// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemiesAI/Pawns/Worm/AIWormCharacter.h"

#include "NiagaraComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AISense_Touch.h"
#include "TraceAndCollision/CustomCollision.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

AAIWormCharacter::AAIWormCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AIType = E_AIType::WORM;
	
	LookAtPos = CreateDefaultSubobject<USceneComponent>("LookAtPos");
	LookAtPos->SetupAttachment(RootComponent);

	HeadCollider = CreateDefaultSubobject<UCapsuleComponent>("Head Collider");
	HeadCollider->SetupAttachment(RootComponent);

	AttackFeedbackVfxLocation = CreateDefaultSubobject<USceneComponent>("Attack Feedback VFX Location");
	AttackFeedbackVfxLocation->SetupAttachment(HeadCollider);

	TerrainVfxLocation = CreateDefaultSubobject<USceneComponent>("Terrain VFX Location");
	TerrainVfxLocation->SetupAttachment(HeadCollider);

	BodyCollider = CreateDefaultSubobject<UCapsuleComponent>("Body Collider");
	BodyCollider->SetupAttachment(HeadCollider);

	TailCollider = CreateDefaultSubobject<UCapsuleComponent>("Tail Collider");
	TailCollider->SetupAttachment(HeadCollider);
}

void AAIWormCharacter::SetCollision(const TArray<TEnumAsByte<ECollisionChannel>>& Channels, bool bIgnore)
{
	UCapsuleComponent* MainCapsule = GetCapsuleComponent();
	TArray<UCapsuleComponent*> CapsuleComponents = { MainCapsule, HeadCollider, BodyCollider, TailCollider };

	for (UCapsuleComponent* Capsule : CapsuleComponents)
	{
		if (!IsValid(Capsule))
		{
			continue;
		}
		
		ECollisionResponse Response = bIgnore ? ECR_Ignore : ECR_Overlap;
		for (ECollisionChannel Channel : Channels)
		{
			Capsule->SetCollisionResponseToChannel(Channel, Response);
		}
	}
}

void AAIWormCharacter::ReportTouchEvent(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor)
		return;

	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(OtherActor);

	if (!Character)
		return;

	UAISense_Touch::ReportTouchEvent(
		GetWorld(),
		this,
		OtherActor,
		OtherActor->GetActorLocation()
	);

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Touch!");
}

void AAIWormCharacter::HandleOverlapEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AStillHearAICharacterBase* OtherCharacter = Cast<AStillHearAICharacterBase>(OtherActor);

	if (OtherCharacter && OtherCharacter != this)
		NearbyAICharacters.Add(OtherCharacter);
}

void AAIWormCharacter::HandleOverlapExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AStillHearAICharacterBase* OtherCharacter = Cast<AStillHearAICharacterBase>(OtherActor);

	if (OtherCharacter)
		NearbyAICharacters.Remove(OtherCharacter);
}

void AAIWormCharacter::HandleHeadHitCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// If I touch the player I tell him to die
	if (OtherComp->GetCollisionObjectType() != ECustomCollision::Player)
	{
		return;
	}
	
	const AStillHearMainCharacter* Character = Cast<AStillHearMainCharacter>(OtherActor);

	if (!IsValid(Character))
		return;
	
	UAbilitySystemComponent* CharacterAsc = Character->GetAbilitySystemComponent();

	if (!IsValid(CharacterAsc))
		return;
	
	const FGameplayEffectSpecHandle SpecHandle = CharacterAsc->MakeOutgoingSpec(
			HitEffectClass,
			0.0f,
			CharacterAsc->MakeEffectContext()
		);

	if (SpecHandle.IsValid())
		CharacterAsc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void AAIWormCharacter::SetLookAtPosLocation(const FVector& NewLookAtPosLocation) const
{
	if (!IsValid(LookAtPos))
		return;
	
	LookAtPos->SetWorldLocation(NewLookAtPosLocation);
}

void AAIWormCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Capture the head's neutral rest rotation only once. BeginPlay re-runs every time the worm's
	// streaming level is made visible again, but by then AdjustCapsuleRotation has left the head in a
	// rotated pose; re-capturing would accumulate that offset and displace all head-attached VFX.
	if (!bCapsuleRotationInitialized)
	{
		InitialCapsuleRotation = HeadCollider->GetComponentRotation();
		bCapsuleRotationInitialized = true;
	}

	HeadCollider->OnComponentHit.AddUniqueDynamic(this, &ThisClass::ReportTouchEvent);
	BodyCollider->OnComponentHit.AddUniqueDynamic(this, &ThisClass::ReportTouchEvent);
	TailCollider->OnComponentHit.AddUniqueDynamic(this, &ThisClass::ReportTouchEvent);

	HeadCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapEnter);
	BodyCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapEnter);
	TailCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapEnter);

	HeadCollider->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapExit);
	BodyCollider->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapExit);
	TailCollider->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlapExit);

	HeadCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleHeadHitCharacter);
}

void AAIWormCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// When this worm's streaming level is hidden, its AIController is destroyed and nobody removes the
	// looping GroundDebris gameplay cue. That leaves the cue notify actor latched (OnRemove never runs)
	// with its VFX attached to a component that is about to unregister, so on the next show the cue can
	// no longer respawn its VFX. Remove the cue here, while the components are still valid, so OnRemove
	// runs and tears the VFX down cleanly; the movement code then re-adds it fresh on the next show.
	if (EndPlayReason == EEndPlayReason::RemovedFromWorld)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RemoveGameplayCue(TAG_GameplayCue_GroundDebris);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AAIWormCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AdjustCapsuleRotation();
	RegulateDistanceWithOtherAICharacters();
}

void AAIWormCharacter::AdjustCapsuleRotation()
{
	const FVector Velocity = GetVelocity();

	if (!Velocity.IsNearlyZero())
	{
		const FVector Direction = Velocity.GetSafeNormal();

		FVector NewLocation = GetActorLocation() + Direction * 150.f;

		LookAtPos->SetWorldLocation(NewLocation);
	}
	
	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(
		GetActorLocation(),
		LookAtPos->GetComponentLocation()
	);

	NewRotation.Pitch = -NewRotation.Pitch;

	const FRotator FinalRotation = InitialCapsuleRotation + NewRotation;
	
	HeadCollider->SetWorldRotation(FinalRotation);
}

void AAIWormCharacter::RegulateDistanceWithOtherAICharacters()
{
	FVector SeparationForce = FVector::ZeroVector;

	for (AActor* Other : NearbyAICharacters)
	{
		FVector Delta = GetActorLocation() - Other->GetActorLocation();
		// float Distance = Delta.Size();

		SeparationForce += Delta.GetSafeNormal() * 3000.f;
		SeparationForce += FVector::CrossProduct(Delta.GetSafeNormal(), FVector::UpVector) * 10000.f;
	}

	AddMovementInput(SeparationForce);
}

