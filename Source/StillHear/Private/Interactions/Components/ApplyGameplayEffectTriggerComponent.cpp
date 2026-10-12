#include "Interactions/Components/ApplyGameplayEffectTriggerComponent.h"

#include "GameplayEffect.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

#pragma region METHODS
void UApplyGameplayEffectTriggerComponent::OnTriggerEnter(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!EffectClass)
		return;

	// Retrieve the ASC from the overlapping actor
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	if (!ASC)
		return;

	// Handle application policies
	if (TriggerPolicy == EEffectTriggerPolicy::ApplyRemove || TriggerPolicy == EEffectTriggerPolicy::ApplyOnly)
	{
	   // Create context and spec for the Gameplay Effect
	   FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	   ContextHandle.AddSourceObject(this);

	   const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);

	   if (SpecHandle.IsValid())
	   {
		  // Apply the effect
		  const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		  
		  // Store the handle only if we need to remove it later on exit
		  if (TriggerPolicy == EEffectTriggerPolicy::ApplyRemove)
			  ActiveEffectHandles.Add(ASC, ActiveHandle);
	   }
	}
	
	// Handle removal policy
	else if (TriggerPolicy == EEffectTriggerPolicy::RemoveOnly)
	{
		// Set up the query to match the specific Effect Class
		FGameplayEffectQuery Query;
		Query.EffectDefinition = EffectClass;
		
		// Remove all active effects matching the query
		ASC->RemoveActiveEffects(Query);
	}
}

void UApplyGameplayEffectTriggerComponent::OnTriggerExit(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	// If the policy doesn't require removal on exit, do nothing
	if (TriggerPolicy != EEffectTriggerPolicy::ApplyRemove)
		return;

	// Retrieve the ASC to find the correct handle
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);

	if (ASC && ActiveEffectHandles.Contains(ASC))
	{
	   // Get the specific handle for this actor
	   const FActiveGameplayEffectHandle HandleToRemove = ActiveEffectHandles[ASC];

	   // Remove the Gameplay Effect
	   ASC->RemoveActiveGameplayEffect(HandleToRemove);

	   // Clean up the map
	   ActiveEffectHandles.Remove(ASC);
	}
}

void UApplyGameplayEffectTriggerComponent::Reset()
{
	Super::Reset();

	for (auto& Pair : ActiveEffectHandles)
	{
		if (UAbilitySystemComponent* ASC = Pair.Key)
		{
			ASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}
	ActiveEffectHandles.Empty();
}
#pragma endregion