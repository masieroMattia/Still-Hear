#include "UI/Widgets/FSRSettingsWidget.h"

#include "HAL/IConsoleManager.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Elements/DropdownBase.h"
#include "UI/Widgets/SettingsRowBase.h"

#pragma region UFUNCTIONS
void UFSRSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeDropdowns();
}

void UFSRSettingsWidget::ApplyFSRSettings()
{
	ApplySuperResolution();
	ApplyFrameGeneration();
}
#pragma endregion

#pragma region METHODS
void UFSRSettingsWidget::ApplySuperResolution()
{
	if (!SuperResolutionDropdown)
		return;

	const int32 SelectedIndex = SuperResolutionDropdown->GetSelectedIndex();
	IConsoleVariable* CVarFSREnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.Enabled"));
	IConsoleVariable* CVarFSRQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FSR.QualityMode"));

	if (CVarFSREnabled && CVarFSRQuality)
	{
		if (SelectedIndex == 0) // NATIVE AA (effectively Off or Native rendering with AA)
		{
			CVarFSREnabled->Set(0, ECVF_SetByGameSetting);
		}
		else
		{
			CVarFSREnabled->Set(1, ECVF_SetByGameSetting);
			CVarFSRQuality->Set(SelectedIndex, ECVF_SetByGameSetting);
		}
	}
}

void UFSRSettingsWidget::ApplyFrameGeneration() const
{
	if (!FrameGenerationDropdown)
		return;

	const int32 SelectedIndex = FrameGenerationDropdown->GetSelectedIndex();
	IConsoleVariable* CVarFIEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.FidelityFX.FI.Enabled"));

	if (CVarFIEnabled)
	{
		CVarFIEnabled->Set(SelectedIndex, ECVF_SetByGameSetting);
	}
}

void UFSRSettingsWidget::SetSavedIndices(const int32 SRIndex, const int32 FGIndex) const
{
	if (SuperResolutionDropdown) SuperResolutionDropdown->SetSelectedIndex(SRIndex);
	if (FrameGenerationDropdown) FrameGenerationDropdown->SetSelectedIndex(FGIndex);
}

int32 UFSRSettingsWidget::GetSuperResolutionIndex() const
{
	return SuperResolutionDropdown ? SuperResolutionDropdown->GetSelectedIndex() : 0;
}

int32 UFSRSettingsWidget::GetFrameGenerationIndex() const
{
	return FrameGenerationDropdown ? FrameGenerationDropdown->GetSelectedIndex() : 0;
}

void UFSRSettingsWidget::InitializeDropdowns()
{
	if (SuperResolutionDropdown)
	{
		const TArray SROptions = {
			FText::FromString("OFF / NATIVE"),
			FText::FromString("QUALITY"),
			FText::FromString("BALANCED"),
			FText::FromString("PERFORMANCE"),
			FText::FromString("ULTRA PERFORMANCE")
		};
		
		SuperResolutionDropdown->SetOptions(SROptions);
		SuperResolutionDropdown->SetSelectedIndex(0);
		SuperResolutionDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UFSRSettingsWidget::HandleSubSettingChanged);
	}
	
	if (FrameGenerationDropdown)
	{
		const TArray FGOptions = {
			FText::FromString("OFF"),
			FText::FromString("ON")
		};
		
		FrameGenerationDropdown->SetOptions(FGOptions);
		FrameGenerationDropdown->SetSelectedIndex(0);
		FrameGenerationDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UFSRSettingsWidget::HandleSubSettingChanged);
	}
}

void UFSRSettingsWidget::HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText)
{	
	OnSettingsChanged.Broadcast();
}

UWidget* UFSRSettingsWidget::GetFirstFocusableChild() const
{
	if (!WidgetTree)
		return nullptr;

	TArray<UWidget*> ChildWidgets;
	WidgetTree->GetAllWidgets(ChildWidgets);

	USettingsRowBase* TopRow = nullptr;
	float TopY = 0.0f;

	for (UWidget* Widget : ChildWidgets)
	{
		USettingsRowBase* Row = Cast<USettingsRowBase>(Widget);
		if (!Row)
			continue;

		const ESlateVisibility Vis = Row->GetVisibility();
		if (Vis != ESlateVisibility::Visible && Vis != ESlateVisibility::SelfHitTestInvisible)
			continue;

		// Pick the visually topmost row
		const float RowY = Row->GetCachedGeometry().GetAbsolutePosition().Y;
		if (!TopRow || RowY < TopY)
		{
			TopRow = Row;
			TopY = RowY;
		}
	}
	return TopRow;
}

UWidget* UFSRSettingsWidget::GetLastFocusableChild() const
{
	if (!WidgetTree)
		return nullptr;

	TArray<UWidget*> ChildWidgets;
	WidgetTree->GetAllWidgets(ChildWidgets);

	USettingsRowBase* BottomRow = nullptr;
	float BottomY = 0.0f;

	for (UWidget* Widget : ChildWidgets)
	{
		USettingsRowBase* Row = Cast<USettingsRowBase>(Widget);
		if (!Row)
			continue;

		const ESlateVisibility Vis = Row->GetVisibility();
		if (Vis != ESlateVisibility::Visible && Vis != ESlateVisibility::SelfHitTestInvisible)
			continue;

		// Pick the visually bottommost row
		const float RowY = Row->GetCachedGeometry().GetAbsolutePosition().Y;
		if (!BottomRow || RowY > BottomY)
		{
			BottomRow = Row;
			BottomY = RowY;
		}
	}
	return BottomRow;
}
#pragma endregion