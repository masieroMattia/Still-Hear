#include "Camera/CameraVolume.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ArrowComponent.h"
#include "Character/StillHearMainCharacter.h"
#include "Interfaces/CameraVolumesInteractor.h"
#include "Character/StillHearPlayerController.h"

ACameraVolume::ACameraVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Volume = CreateDefaultSubobject<UBoxComponent>(FName("Volume"));
	RootComponent = Volume;

	OuterVolume = CreateDefaultSubobject<UBoxComponent>(FName("OuterVolume"));
	OuterVolume->SetVisibility(false);
	OuterVolume->SetupAttachment(Volume);

	OuterVolume->SetCollisionObjectType(ECC_WorldDynamic);
	OuterVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
	OuterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OuterVolume->SetGenerateOverlapEvents(true);
	Volume->SetCollisionObjectType(ECC_WorldDynamic);
	Volume->SetCollisionResponseToAllChannels(ECR_Overlap);
	Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Volume->SetGenerateOverlapEvents(true);

	Volume->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::BeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &ThisClass::EndOverlap);
	
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(Volume);

	InputArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RightInputDirection"));
	InputArrow->SetupAttachment(Volume);

}

UCameraComponent* ACameraVolume::GetCamera() const
{
	return CameraComponent;
}

bool ACameraVolume::ContainsPoint(const FVector& WorldLocation) const
{
	if (!Volume)
		return false;
	
	const FVector Local = Volume->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = Volume->GetScaledBoxExtent();
	
	return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y && FMath::Abs(Local.Z) <= Extent.Z;
}

void ACameraVolume::Activate(AActor* Actor)
{
	ActorInVolume = Actor;
	SetActorTickEnabled(true);
}

void ACameraVolume::Deactivate()
{
	ActorInVolume = nullptr;
	SetActorTickEnabled(false);
}

void ACameraVolume::RequestSnapToTarget()
{
	bShouldSnapToTarget = true;
}

float ACameraVolume::GetBlendTimeOnEnter() const
{
	return BlendTimeOnEnter;
}

TEnumAsByte<EViewTargetBlendFunction> ACameraVolume::GetBlendFunction() const
{
	return BlendFunction;
}

float ACameraVolume::GetBlendExp() const
{
	return BlendExp;
}

FVector ACameraVolume::GetRightDirection() const
{
	return InputArrow->GetForwardVector();
}

bool ACameraVolume::GetUseBlendParametersOnExit() const
{
	return bUseBlendParametersOnExit;
}

float ACameraVolume::GetBlendTimeOnExit() const
{
	return BlendTimeOnExit;
}

int ACameraVolume::GetPriority() const
{
	return Priority;
}


void ACameraVolume::BeginPlay()
{
	Super::BeginPlay();
	
	ActorInVolume = nullptr;

	if (CameraComponent)
		DefaultCameraTransform = CameraComponent->GetRelativeTransform();
}

void ACameraVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!Volume || !OuterVolume)
		return;

	const FVector VolumeExtent = Volume->GetScaledBoxExtent();

	OuterVolume->SetBoxExtent(VolumeExtent + FVector(10.0f));

	OuterVolume->SetRelativeLocation(FVector::ZeroVector);
	OuterVolume->SetRelativeRotation(Volume->GetRelativeRotation());
}

void ACameraVolume::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->Implements<UCameraVolumesInteractor>())
	{
		ICameraVolumesInteractor::Execute_AddCameraVolumeToList(OtherActor, this);
	}
}

void ACameraVolume::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->Implements<UCameraVolumesInteractor>())
	{
		ICameraVolumesInteractor::Execute_RemoveCameraVolumeFromList(OtherActor, this);


		bHasPlayerAdjustedToInput = false;
	}
}

void ACameraVolume::UpdateCamera(FVector TargetPoint, float DeltaTime)
{
	if (!bLookAtPlayer || !ActorInVolume || !ActorInVolume->Implements<UCameraVolumesInteractor>())
		return;

	const ACharacter* Character = Cast<ACharacter>(ActorInVolume);
	const AStillHearPlayerController* PlayerController = Cast<AStillHearPlayerController>(Character->GetController());
	
	const FRotator StartRotation = CameraComponent->GetComponentRotation();
	const FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(CameraComponent->GetComponentLocation(), PlayerController->GetCharacter()->GetActorLocation());
	
	if (bShouldSnapToTarget)
	{
		// Instant rotation snap (no interpolation)
		CameraComponent->SetWorldRotation(DesiredRotation);
	}
	else
	{
		// Smooth rotation
		const FRotator SmoothRotation = FMath::RInterpTo(StartRotation, DesiredRotation, DeltaTime, RotationSpeed);
		CameraComponent->SetWorldRotation(SmoothRotation);
	}
}

void ACameraVolume::UpdateInputArrow()
{
	if (!bInputFollowsCamera)
		return;

	const FVector TargetDirection = CameraComponent->GetRightVector();
	const FRotator Rotation = FRotationMatrix::MakeFromX(TargetDirection).Rotator();
	InputArrow->SetWorldRotation(Rotation);
}


void ACameraVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!ActorInVolume)
		return;

	UpdateInputArrow();
	UpdateCamera(ICameraVolumesInteractor::Execute_GetTargetPointLocation(ActorInVolume), DeltaTime);

	// Reset the snap flag if this is the base class
	// Derived classes (e.g. FollowSplineCamera) will reset it after applying the position
	if (GetClass() == ACameraVolume::StaticClass())
	{
		bShouldSnapToTarget = false;
	}
}

void ACameraVolume::ResetToDefaultTransform()
{
	if (CameraComponent)
	{
		CameraComponent->SetRelativeTransform(DefaultCameraTransform);
		bShouldSnapToTarget = true;
	}
	Deactivate();
}
