#include "Character/StillHearCompanionCharacter.h"

#include "Character/StillHearCompanionAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"

// Sets default values
AStillHearCompanionCharacter::AStillHearCompanionCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// I set the main mesh to be hidden at the beginning because I will use the three "status meshes" to show the different statuses of the companion,
	// so I don't need the main one. I just need it to be able to use the animations and the animation blueprint, but I will hide it and show only the mesh of the current status.
	GetMesh()->SetHiddenInGame(true);
	
	// I set every mesh to be hidden at the beginning because I will decide which one to show at begin play depending
	// on the "current status" (which in this case is the starting one)
	JoyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Joyful Mesh"));
	JoyMeshComponent->SetupAttachment(RootComponent);
	JoyMeshComponent->SetHiddenInGame(true);
	
	AngerMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Angry Mesh"));
	AngerMeshComponent->SetupAttachment(RootComponent);
	AngerMeshComponent->SetHiddenInGame(true);

	FearMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Scared Mesh"));
	FearMeshComponent->SetupAttachment(RootComponent);
	FearMeshComponent->SetHiddenInGame(true);
	
	AIControllerClass = AStillHearCompanionAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Add the Attribute Set
	AttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("AttributeSet"));

	GetCharacterMovement()->MovementMode = MOVE_Flying;
	GetCharacterMovement()->BrakingDecelerationFlying = 2048.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

	// Initialize floating parameters
	const float CurrentFloorHeight = CheckFloorHeight();
	TargetFloatingHeight = CurrentFloorHeight + FloatingHeight;
	FloatingCycleElapsedTime = 0.0f;
	bReadjusting = false;
}

// Called when the game starts or when spawned
void AStillHearCompanionCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeStats();
	
	AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Status_Companion_Aiming).AddUObject(this, &ThisClass::HandleAiming);

	// I create a dynamic material instance to be able to change the color of the material at runtime depending on the current status.
	// I set the same material instance for all the meshes because they all have the same material,
	// so I can just change the color of that one material instance, and it will affect all the meshes.
	// I take the material of the Joy mesh, but it doesn't matter which one I take because they all have the same material.
	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(JoyMeshComponent->GetMaterial(0), this);
	JoyMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	AngerMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	FearMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	// Here I set the starting status. For now I set it to Joy, but I can change it later if I want to start with a different one.
	CurrentStatus = ECompanionStatus::Joy;
	SetStatus(ECompanionStatus::Joy);
}

// Called every frame
void AStillHearCompanionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Levitate();
	Rotate();
}

void AStillHearCompanionCharacter::InitializeStats() const
{
	if (!AbilitySystemComponent)
		return;

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		StatsInitializerGameplayEffectClass,
		1.0f,
		AbilitySystemComponent->MakeEffectContext()
	);

	if (!SpecHandle.IsValid())
		return;

	// Initialize all the stats here by setting their attribute value through the "value initializer" gameplay effect
	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.BaseSpeed")),
		BaseSpeed
	);

	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.SpeedMultiplier")),
		1.0f
	);

	// Apply the effect to change the values in the character class
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void AStillHearCompanionCharacter::StartSoundWave() const
{
	if (!AbilitySystemComponent)
		return;
	
	AbilitySystemComponent->TryActivateAbilityByClass(SoundWaveAbilityClass);
}

void AStillHearCompanionCharacter::ShootSoundWave()
{
	if (!AbilitySystemComponent)
		return;
	
	const FGameplayTag EventTag = TAG_Event_Companion_ShootSoundWave;
	SendGameplayEventToSelf(EventTag);
}

void AStillHearCompanionCharacter::InterruptSoundWave()
{
	if (!AbilitySystemComponent)
		return;
	
	FGameplayTagContainer AbilityTagsToCancel;
	AbilityTagsToCancel.AddTag(TAG_GameplayAbility_Companion_SoundWave);
	
	AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
}

float AStillHearCompanionCharacter::CheckFloorHeight() const
{
	// The goal is to have a raycast that goes from the bottom of the capsule, down to the floor.
	// Hit Result declaration
	FHitResult HitResult;

	// Start Vector
	FVector StartLocation = GetCapsuleComponent()->GetComponentLocation();
	StartLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// End Vector
	// I set the end location to be 2000 units below the start location, which should be enough to hit the floor in most cases.
	// If the raycast doesn't hit anything, I can be sure that I'm not close to the floor.
	const FVector EndLocation = StartLocation + FVector::DownVector * 2000.0f; 

	// Ignore Parameters
	FCollisionQueryParams IgnoreParams;
	IgnoreParams.AddIgnoredActor(this);
	
	// Line Trace by Channel
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,  
		IgnoreParams
	);

	if (bShowTrace)
		DrawDebugRayCast(StartLocation, EndLocation, bHit, HitResult);

	float CurrentFloorZ = 0.0f;
	
	if (bHit)
	{
		CurrentFloorZ = HitResult.ImpactPoint.Z;
	}
	else
	{
		CurrentFloorZ = -1.0f; // If the raycast doesn't hit, I set the distance to -1 to know that I'm not close to the floor at all
	}

	return CurrentFloorZ;
}

bool AStillHearCompanionCharacter::IsNotInFloatingRange(const float& CurrentDistanceFromFloor) const
{
	return	CurrentDistanceFromFloor == -1.0f ||
			CurrentDistanceFromFloor < FloatingHeight - FloatingHeightOffset ||
			CurrentDistanceFromFloor > FloatingHeight + FloatingHeightOffset;
}

void AStillHearCompanionCharacter::DrawDebugRayCast(const FVector& Start, const FVector& End, const bool Hit, const FHitResult& HitResult) const
{
	
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		Hit ? FColor::Green : FColor::Red,
		false,      
		5.0f,       
		0,
		2.0f        
	);

	if (Hit)
	{
		// Point of the impact
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, 5.0f);

		// Normal of the impact
		DrawDebugDirectionalArrow(
			GetWorld(),
			HitResult.ImpactPoint,
			HitResult.ImpactPoint + HitResult.ImpactNormal * 50.0f,
			20.0f,
			FColor::Cyan,
			false,
			1.0f,
			0,
			2.0f
		);
	}
}

void AStillHearCompanionCharacter::HandleAiming(const FGameplayTag AimTag, int32 NewCount)
{
	
}

void AStillHearCompanionCharacter::Levitate()
{
	const float CurrentFloorHeight = CheckFloorHeight();
	const float NewFloatingHeight = CurrentFloorHeight + FloatingHeight;
	const FVector CurrentLocation = GetActorLocation();

	if (NewFloatingHeight != TargetFloatingHeight)
	{
		TargetFloatingHeight = NewFloatingHeight;
		bFloat = false; // I need to stop floating to adjust the height
	}
	
	if (bFloat)
	{
		// I lerp the floating height to create a smooth up and down movement
		FloatingCycleElapsedTime += GetWorld()->GetDeltaSeconds();
		FloatingCycleElapsedTime = FMath::Fmod(FloatingCycleElapsedTime, FloatingCycleTime); // Loop the cycle time
		float NewZ = FMath::Sin(FloatingCycleElapsedTime / FloatingCycleTime * 2 * PI) * FloatingHeightOffset;
		NewZ = FMath::Clamp(NewZ, -FloatingHeightOffset + 0.01f, FloatingHeightOffset - 0.01f); // Clamp the value to avoid going out of the desired range due to floating point inaccuracies
		FVector NewLocation = CurrentLocation;
		NewLocation.Z = TargetFloatingHeight + NewZ;
		SetActorLocation(NewLocation);
	}
	else
	{
		if (!bReadjusting ||
			(TargetLocationZ != TargetFloatingHeight - FloatingHeightOffset && TargetLocationZ != TargetFloatingHeight + FloatingHeightOffset))
		{
			InitialLocationZ = CurrentLocation.Z;
			TargetLocationZ = TargetFloatingHeight;
			if (TargetLocationZ > InitialLocationZ)
				TargetLocationZ += FloatingHeightOffset;
			else
				TargetLocationZ -= FloatingHeightOffset;
			bReadjusting = true;
		}
		
		// If I'm not floating, I need to adjust the vertical position in a smooth way so that I can start floating again.
		const float NewZ = FMath::FInterpTo(CurrentLocation.Z, TargetLocationZ, GetWorld()->GetDeltaSeconds(), 3.f);
		FVector NewLocation = CurrentLocation;
		NewLocation.Z = NewZ;
		SetActorLocation(NewLocation);
		// If I'm close enough to the target distance from the floor, I can start floating again.
		if (FMath::IsNearlyEqual(NewZ, TargetLocationZ, 2.f))
		{
			bFloat = true;
			bReadjusting = false;
			FloatingCycleElapsedTime = TargetLocationZ > InitialLocationZ  ? FloatingCycleTime * 0.17f : FloatingCycleTime * 0.82f; // Start the cycle at the right point to avoid sudden jumps in the movement
		}
	}
}

void AStillHearCompanionCharacter::Rotate()
{
	if (!bRotateOnItself)
		return;
	
	// Rotate the character on itself
	const FRotator RotationDelta(0.f, RotationSpeed * GetWorld()->GetDeltaSeconds(), 0.f);
	GetCurrentMesh()->AddLocalRotation(RotationDelta);
}

USkeletalMeshComponent* AStillHearCompanionCharacter::GetCurrentMesh() const
{
	if (CurrentStatus == ECompanionStatus::Joy)
		return JoyMeshComponent;
	
	if (CurrentStatus == ECompanionStatus::Anger)
		return AngerMeshComponent;

	// If I'm here, it means that the current status is Fear, because there are only three statuses and I already checked the other two.
	// I remove the check because if I don't the compiler will complain that not all paths return a value.
	
	//if (CurrentStatus == ECompanionStatus::Fear)
		return FearMeshComponent;
}

void AStillHearCompanionCharacter::SetStatus(const ECompanionStatus NewStatus)
{
	AngerMeshComponent->SetHiddenInGame(true);
	FearMeshComponent->SetHiddenInGame(true);
	JoyMeshComponent->SetHiddenInGame(true);
	
	if (NewStatus == ECompanionStatus::Joy)
	{		
		JoyMeshComponent->SetHiddenInGame(false);
		DynamicMaterialInstance->SetVectorParameterValue(FName("MainColor"), JoyColor);
		CurrentStatus = ECompanionStatus::Joy;
	}
	else if (NewStatus == ECompanionStatus::Anger)
	{
		AngerMeshComponent->SetHiddenInGame(false);
		DynamicMaterialInstance->SetVectorParameterValue(FName("MainColor"), AngerColor);
		CurrentStatus = ECompanionStatus::Anger;
	}
	else if (NewStatus == ECompanionStatus::Fear)
	{
		FearMeshComponent->SetHiddenInGame(false);
		DynamicMaterialInstance->SetVectorParameterValue(FName("MainColor"), FearColor);
		CurrentStatus = ECompanionStatus::Fear;
	}
}

void AStillHearCompanionCharacter::HandleDeath(const FGameplayTag DeathTag, int32 NewCount)
{
	if (NewCount <= 0)
		return;
	
	Super::HandleDeath(DeathTag, NewCount);
	OnCompanionDeathDelegate.Broadcast();
}
