#include "UI/AI/EnemyStatus.h"

#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "UI/Indicator/IndicatorDescriptor.h"
#include "GameplayAbilitySystem/Tags/GameplayTags.h"
#include "EnemiesAI/Utility/Components/PerceptionsMeterComponent.h"

#pragma region METHODS
void UEnemyStatus::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!Descriptor || !Descriptor->TargetActor)
		return;

	const AStillHearAICharacterBase* Enemy = Cast<AStillHearAICharacterBase>(Descriptor->TargetActor);
	if (!Enemy)
		return;

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Descriptor->TargetActor))
		CachedASC = ASI->GetAbilitySystemComponent();

	CachedAIC = Cast<AStillHearAIControllerBase>(Enemy->GetController());
	if (CachedAIC.IsValid())
	{
		CachedAIC->OnStatusTagChanged.AddUniqueDynamic(this, &UEnemyStatus::UpdateStatus);

		// Cache data from the map
		CachedAlertData = FindStatusData(TAG_Status_EnemyAI_Alerted);
		CachedSuspiciousData = FindStatusData(TAG_Status_EnemyAI_Suspicious);

		// Try to find the Perception Meter
		UPerceptionsMeterComponent* MeterComp = CachedAIC->FindComponentByClass<UPerceptionsMeterComponent>();
		if (IsValid(MeterComp))
		{
			// If it exists, bind to the gradual filling event
			MeterComp->OnPerceptionsUpdated.AddUniqueDynamic(this, &UEnemyStatus::UpdateMeter);
		}

		// Initial update to synchronize with the current state of the AI
		UpdateStatus(CachedAIC->GetCurrentStatusTag());
	}
}
#pragma endregion
	
#pragma region UFUNCTIONS
void UEnemyStatus::UpdateMeter(const float AwarenessPercent, const float AlertPercent)
{
	if (!Descriptor || !Descriptor->TargetActor)
		return;

	// Priority Check: If the enemy is attacking, don't let the meter overwrite the UI
	if (IsValid(CachedASC) && CachedASC->HasMatchingGameplayTag(TAG_GameplayAbility_EnemyAI_Attack_Active))
	{
		return;
	}

	const float CurrentFill = (AlertPercent > 0.0f) ? AlertPercent : AwarenessPercent;
	
	if (CurrentFill <= 0.01f)
	{
		if (CachedAIC.IsValid())
		{
			UpdateStatus(CachedAIC->GetCurrentStatusTag());
		}
		else if (IsValid(StatusIconBar))
		{
			StatusIconBar->SetRenderOpacity(0.0f);
		}
		return;
	}

	const FEnemyStatusData* ActiveData = (AlertPercent > 0.0f) ? CachedAlertData : CachedSuspiciousData;
	UpdateWidgetVisuals(ActiveData, CurrentFill);
}

void UEnemyStatus::UpdateStatus(const FGameplayTag NewStatusTag)
{
	const FEnemyStatusData* Data = FindStatusData(NewStatusTag);
	if (Data)
	{
		UpdateWidgetVisuals(Data, Data->bIsVisible ? 1.0f : 0.0f);
	}
	else if (IsValid(StatusIconBar))
	{
		StatusIconBar->SetPercent(0.0f);
		StatusIconBar->SetRenderOpacity(0.0f);
	}
}

const FEnemyStatusData* UEnemyStatus::FindStatusData(const FGameplayTag& Tag) const
{
	if (const FEnemyStatusData* FoundData = StatusConfig.Find(Tag))
	{
		return FoundData;
	}

	if (IsValid(CachedASC))
	{
		for (const auto& Pair : StatusConfig)
		{
			if (CachedASC->HasMatchingGameplayTag(Pair.Key))
			{
				return &Pair.Value;
			}
		}
	}

	return nullptr;
}

void UEnemyStatus::UpdateWidgetVisuals(const FEnemyStatusData* Data, const float Percent) const
{
	if (!Data || !IsValid(StatusIconBar))
		return;

	if (Descriptor)
	{
		Descriptor->bClampToScreen = Data->bClampToScreen;
		Descriptor->bShowOnlyOffScreen = Data->bShowOnlyOffScreen;
	}

	StatusIconBar->SetRenderOpacity(Data->bIsVisible ? 1.0f : 0.0f);

	const float FinalPercent = (Data->bAlwaysFull && Percent > 0.0f) ? 1.0f : Percent;
	StatusIconBar->SetPercent(FinalPercent);

	FLinearColor FinalColor = Data->Color;
	FinalColor.A = 1.0f;
	StatusIconBar->SetFillColorAndOpacity(FinalColor);

	FProgressBarStyle Style = StatusIconBar->GetWidgetStyle();
	if (Style.FillImage.GetResourceObject() != Data->Icon)
	{
		Style.FillImage.SetResourceObject(Data->Icon);
		StatusIconBar->SetWidgetStyle(Style);
	}
}
#pragma endregion
