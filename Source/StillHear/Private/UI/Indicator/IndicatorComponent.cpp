#include "UI/Indicator/IndicatorComponent.h"

#include "UI/Indicator/IndicatorSubsystem.h"
#include "UI/Indicator/IndicatorDescriptor.h"
#include "UI/Indicator/IndicatorWidgetBase.h"

#pragma region CONSTRUCTOR
UIndicatorComponent::UIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
#pragma endregion

#pragma region METHODS
void UIndicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegister)
		RegisterIndicator();
}

void UIndicatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UnregisterIndicator();
}
#pragma endregion

#pragma region UFUNCTIONS
void UIndicatorComponent::RegisterIndicator()
{
	// Prevent double registration
	if (IndicatorDescriptor)
		return;

	if (IndicatorClass && GetWorld())
	{
		IndicatorDescriptor = NewObject<UIndicatorDescriptor>(this);
		IndicatorDescriptor->TargetActor = GetOwner();
		IndicatorDescriptor->IndicatorWidgetClass = IndicatorClass;
		IndicatorDescriptor->WorldOffset = WorldOffset;
		IndicatorDescriptor->bClampToScreen = bClampToScreen;
		IndicatorDescriptor->bShowOnlyOffScreen = bShowOnlyOffScreen;
		IndicatorDescriptor->bCheckOcclusion = bCheckOcclusion;
		IndicatorDescriptor->InputActions = InputActions;
		IndicatorDescriptor->SeparatorClass = SeparatorClass;

		UIndicatorSubsystem* Subsystem = GetWorld()->GetSubsystem<UIndicatorSubsystem>();
		if (Subsystem)
			Subsystem->AddIndicator(IndicatorDescriptor);
	}
}

void UIndicatorComponent::UnregisterIndicator()
{
	if (IndicatorDescriptor && GetWorld())
	{
		UIndicatorSubsystem* Subsystem = GetWorld()->GetSubsystem<UIndicatorSubsystem>();
		if (Subsystem)
			Subsystem->RemoveIndicator(IndicatorDescriptor);

		// Clear the pointer to allow future re-registration
		IndicatorDescriptor = nullptr;
	}
}

void UIndicatorComponent::UpdatePromptActions(const TArray<UInputAction*>& NewActions, TSubclassOf<class UUserWidget> NewSeparatorClass)
{
	InputActions = NewActions;
	if (NewSeparatorClass)
	{
		SeparatorClass = NewSeparatorClass;
	}
	
	if (IndicatorDescriptor)
	{
		IndicatorDescriptor->InputActions = InputActions;
		IndicatorDescriptor->SeparatorClass = SeparatorClass;
		IndicatorDescriptor->OnIndicatorUpdated.Broadcast();
	}
}
#pragma endregion