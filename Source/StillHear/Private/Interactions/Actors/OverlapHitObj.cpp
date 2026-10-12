#include "Interactions/Actors/OverlapHitObj.h"

#include "AbilitySystemGlobals.h"
#include "StillHearGameInstance.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"

AOverlapHitObj::AOverlapHitObj()
{
	ProximitySphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphereComponent"));
	ProximitySphereComponent->SetupAttachment(RootComponent);
	ProximitySphereComponent->SetGenerateOverlapEvents(true);
	ProximitySphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProximitySphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}

#pragma region OVERRIDE METHODS
void AOverlapHitObj::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	if (ProximitySphereComponent)
	{
		for (const TEnumAsByte Channel : CollisionChannels)
			ProximitySphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
		
		ProximitySphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AOverlapHitObj::OnProximityBeginOverlap);
		ProximitySphereComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &AOverlapHitObj::OnProximityEndOverlap);
	}

	StartTimerForOverlapCheck();
}

void AOverlapHitObj::HitTarget(const int32 DeflectionsCount)
{
	if (InteractableTags.IsEmpty())
		return;
	
	Super::HitTarget();
	
	StartInteraction(); // Start the interaction when hit by a projectile, regardless of the number of deflections
}

void AOverlapHitObj::HandleTimelineFinished()
{
	ACharacter* Character = Cast<ACharacter>(CurrentInteractor);
	if (Character)
	{
		// Snap the character to the center of the object, keeping the character's Z to avoid clipping
		FVector SnapLocation = GetActorLocation();
		SnapLocation.Z = Character->GetActorLocation().Z;
		Character->SetActorLocation(SnapLocation);
		
		// Apply the hit effect to the interactor
		if (HitEffectClass)
		{
			UAbilitySystemComponent* InteractorAsc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
			if (InteractorAsc)
			{
				const FGameplayEffectSpecHandle SpecHandle = InteractorAsc->MakeOutgoingSpec(
					HitEffectClass,
					0.0f,
					InteractorAsc->MakeEffectContext()
				);
				
				if (SpecHandle.IsValid())
					InteractorAsc->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}
	}
	
	GetWorldTimerManager().ClearTimer(OverlapCheckTimerHandle);
	
	InteractableTags.Reset(); // Remove all tags so the object is no longer interactable
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore); // Disable all collisions to prevent further interactions	
	
	if (ProximitySphereComponent)
	{
		ProximitySphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	Super::HandleTimelineFinished();
}

void AOverlapHitObj::Reset()
{
	Super::Reset();
	
	bool bForceNewGame = false;
	if (const UWorld* World = GetWorld())
	{
		if (const UStillHearGameInstance* GI = World->GetGameInstance<UStillHearGameInstance>())
		{
			bForceNewGame = GI->bIsNewGameResetting;
		}
	}

	if (!bResetObj && !bForceNewGame)
		return;
	
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	if (ProximitySphereComponent)
	{
		for (const TEnumAsByte Channel : CollisionChannels)
			ProximitySphereComponent->SetCollisionResponseToChannel(Channel, ECR_Overlap);
	}
	
	StartTimerForOverlapCheck();
}
#pragma endregion

#pragma region METHODS
void AOverlapHitObj::StartTimerForOverlapCheck()
{
	GetWorldTimerManager().SetTimer(
		OverlapCheckTimerHandle,
		this,
		&AOverlapHitObj::CheckForOverlappingCharacter,
		OverlapCheckInterval,
		true // looping
	);
}

void AOverlapHitObj::CheckForOverlappingCharacter()
{
	if (!SphereComponent)
		return;
	
	TArray<AActor*> OverlappingActors;
	SphereComponent->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
	
	// If we have an interactor, check if they're still inside
	if (CurrentInteractor && !OverlappingActors.Contains(CurrentInteractor))
		CurrentInteractor = nullptr;
	
	// If no interactor, check if a character entered
	if (!CurrentInteractor)
	{
		for (AActor* Actor : OverlappingActors)
		{
			ACharacter* Character = Cast<ACharacter>(Actor);
			if (Character)
			{
				CurrentInteractor = Character;
				StartInteraction(Character);
				break;
			}
		}
	}
}

void AOverlapHitObj::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PlayProximitySound(OtherActor);
	PlayProximityVFX(OtherActor);
}

void AOverlapHitObj::OnProximityEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	StopProximitySound(OtherActor);
	StopProximityVFX(OtherActor);
}
#pragma endregion