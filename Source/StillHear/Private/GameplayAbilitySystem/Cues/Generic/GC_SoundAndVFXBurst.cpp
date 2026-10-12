#include "GameplayAbilitySystem/Cues/Generic/GC_SoundAndVFXBurst.h"

#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#pragma region CONSTRUCTOR
UGC_SoundAndVFXBurst::UGC_SoundAndVFXBurst()
{
   // Initialize default values
   SpawnedOnCharacter = true;
    
   VFX = nullptr;
   VFXAttachPoint = EAttachPoint::ROOT;
   VFXSocketName = NAME_None;
   VFXComponentTag = NAME_None;
    
   SFX = nullptr;
   SFXAttachPoint = EAttachPoint::ROOT;
   SFXSocketName = NAME_None;
}
#pragma endregion

#pragma region METHODS
void UGC_SoundAndVFXBurst::SpawnBurstEffects(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
    if (!MyTarget) return;

    USceneComponent* TargetSFXComponent = MyTarget->GetRootComponent();
    USceneComponent* TargetVFXComponent = MyTarget->GetRootComponent();

    if (SpawnedOnCharacter)
    {
       const ACharacter* MyCharacter = Cast<ACharacter>(MyTarget);
       
       // Handle SFX Component Selection
       if (SFXAttachPoint == EAttachPoint::MESH && MyCharacter)
          TargetSFXComponent = MyCharacter->GetMesh();
       else if (SFXAttachPoint == EAttachPoint::COMPONENT && SFXComponentTag != NAME_None)
       {
          TArray<USceneComponent*> Components;
          MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
          for (USceneComponent* Comp : Components)
          {
             if (Comp->ComponentHasTag(SFXComponentTag))
             {
                TargetSFXComponent = Comp;
                break;
             }
          }
       }

       // Handle VFX Component Selection
       if (VFXAttachPoint == EAttachPoint::MESH && MyCharacter)
          TargetVFXComponent = MyCharacter->GetMesh();
       else if (VFXAttachPoint == EAttachPoint::COMPONENT && VFXComponentTag != NAME_None)
       {
          TArray<USceneComponent*> Components;
          MyTarget->GetComponents(USceneComponent::StaticClass(), Components);
          for (USceneComponent* Comp : Components)
          {
             if (Comp->ComponentHasTag(VFXComponentTag))
             {
                TargetVFXComponent = Comp;
                break;
             }
          }
       }
    }

    // Determine spawning logic based on provided location in Parameters
    const bool bHasLocation = !Parameters.Location.IsNearlyZero();

    if (SFX)
    {
       if (bHasLocation)
       {
          UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SFX, Parameters.Location);
       }
       else if (TargetSFXComponent)
       {
          UGameplayStatics::SpawnSoundAttached(SFX, TargetSFXComponent, SFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
       }
    }

    if (VFX)
    {
       if (bHasLocation)
       {
          FRotator SpawnRotation = Parameters.Normal.IsNearlyZero() ? MyTarget->GetActorRotation() : Parameters.Normal.Rotation();
          UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFX, Parameters.Location, SpawnRotation);
       }
       else if (TargetVFXComponent)
       {
          UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, TargetVFXComponent, VFXSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
       }
    }
}

bool UGC_SoundAndVFXBurst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
    if (!MyTarget)
       return false;

    SpawnBurstEffects(MyTarget, Parameters);

    return Super::OnExecute_Implementation(MyTarget, Parameters);
}
#pragma endregion