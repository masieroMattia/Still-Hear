#include "UI/Widgets/CustomQualitySettingsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "UI/Elements/DropdownBase.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "GameFramework/GameUserSettings.h"

#pragma region CONSTRUCTOR
void UCustomQualitySettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeDropdowns();
}
#pragma endregion

#pragma region UFUNCTIONS
void UCustomQualitySettingsWidget::HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText)
{
	OnSettingsChanged.Broadcast();
}
#pragma endregion

#pragma region METHODS
void UCustomQualitySettingsWidget::ApplyCustomQualitySettings(UGameUserSettings* UserSettings)
{
	if (!UserSettings)
		return;

	if (ViewDistanceDropdown) UserSettings->SetViewDistanceQuality(ViewDistanceDropdown->GetSelectedIndex());
	if (AntiAliasingDropdown) UserSettings->SetAntiAliasingQuality(AntiAliasingDropdown->GetSelectedIndex());
	if (ShadowQualityDropdown) UserSettings->SetShadowQuality(ShadowQualityDropdown->GetSelectedIndex());
	if (GlobalIlluminationDropdown) UserSettings->SetGlobalIlluminationQuality(GlobalIlluminationDropdown->GetSelectedIndex());
	if (ReflectionQualityDropdown) UserSettings->SetReflectionQuality(ReflectionQualityDropdown->GetSelectedIndex());
	if (TextureQualityDropdown) UserSettings->SetTextureQuality(TextureQualityDropdown->GetSelectedIndex());
	if (EffectsQualityDropdown) UserSettings->SetVisualEffectQuality(EffectsQualityDropdown->GetSelectedIndex());
	if (PostProcessingDropdown) UserSettings->SetPostProcessingQuality(PostProcessingDropdown->GetSelectedIndex());
	if (FoliageQualityDropdown) UserSettings->SetFoliageQuality(FoliageQualityDropdown->GetSelectedIndex());
	if (ShadingQualityDropdown) UserSettings->SetShadingQuality(ShadingQualityDropdown->GetSelectedIndex());
}

void UCustomQualitySettingsWidget::RefreshVisuals(const UGameUserSettings* UserSettings)
{
	if (!UserSettings)
		return;

	if (ViewDistanceDropdown) ViewDistanceDropdown->SetSelectedIndex(UserSettings->GetViewDistanceQuality());
	if (AntiAliasingDropdown) AntiAliasingDropdown->SetSelectedIndex(UserSettings->GetAntiAliasingQuality());
	if (ShadowQualityDropdown) ShadowQualityDropdown->SetSelectedIndex(UserSettings->GetShadowQuality());
	if (GlobalIlluminationDropdown) GlobalIlluminationDropdown->SetSelectedIndex(UserSettings->GetGlobalIlluminationQuality());
	if (ReflectionQualityDropdown) ReflectionQualityDropdown->SetSelectedIndex(UserSettings->GetReflectionQuality());
	if (TextureQualityDropdown) TextureQualityDropdown->SetSelectedIndex(UserSettings->GetTextureQuality());
	if (EffectsQualityDropdown) EffectsQualityDropdown->SetSelectedIndex(UserSettings->GetVisualEffectQuality());
	if (PostProcessingDropdown) PostProcessingDropdown->SetSelectedIndex(UserSettings->GetPostProcessingQuality());
	if (FoliageQualityDropdown) FoliageQualityDropdown->SetSelectedIndex(UserSettings->GetFoliageQuality());
	if (ShadingQualityDropdown) ShadingQualityDropdown->SetSelectedIndex(UserSettings->GetShadingQuality());
}

bool UCustomQualitySettingsWidget::GetIsDirty(const UGameUserSettings* UserSettings) const
{
	if (!UserSettings)
		return false;

	if (ViewDistanceDropdown && ViewDistanceDropdown->GetSelectedIndex() != UserSettings->GetViewDistanceQuality()) return true;
	if (AntiAliasingDropdown && AntiAliasingDropdown->GetSelectedIndex() != UserSettings->GetAntiAliasingQuality()) return true;
	if (ShadowQualityDropdown && ShadowQualityDropdown->GetSelectedIndex() != UserSettings->GetShadowQuality()) return true;
	if (GlobalIlluminationDropdown && GlobalIlluminationDropdown->GetSelectedIndex() != UserSettings->GetGlobalIlluminationQuality()) return true;
	if (ReflectionQualityDropdown && ReflectionQualityDropdown->GetSelectedIndex() != UserSettings->GetReflectionQuality()) return true;
	if (TextureQualityDropdown && TextureQualityDropdown->GetSelectedIndex() != UserSettings->GetTextureQuality()) return true;
	if (EffectsQualityDropdown && EffectsQualityDropdown->GetSelectedIndex() != UserSettings->GetVisualEffectQuality()) return true;
	if (PostProcessingDropdown && PostProcessingDropdown->GetSelectedIndex() != UserSettings->GetPostProcessingQuality()) return true;
	if (FoliageQualityDropdown && FoliageQualityDropdown->GetSelectedIndex() != UserSettings->GetFoliageQuality()) return true;
	if (ShadingQualityDropdown && ShadingQualityDropdown->GetSelectedIndex() != UserSettings->GetShadingQuality()) return true;

	return false;
}

void UCustomQualitySettingsWidget::InitializeDropdowns()
{
	const TArray QualityOptions = {
		FText::FromString("LOW"),
		FText::FromString("MEDIUM"),
		FText::FromString("HIGH"),
		FText::FromString("EPIC"),
		FText::FromString("CINEMATIC")
	};

	auto InitHelper = [this, &QualityOptions](UDropdownBase* Dropdown)
	{
		if (Dropdown)
		{
			Dropdown->SetOptions(QualityOptions);
			Dropdown->SetSelectedIndex(2); // Default to HIGH (2)
			Dropdown->OnSelectionChanged.RemoveAll(this);
			Dropdown->OnSelectionChanged.AddUniqueDynamic(this, &UCustomQualitySettingsWidget::HandleSubSettingChanged);
		}
	};

	InitHelper(ViewDistanceDropdown);
	InitHelper(AntiAliasingDropdown);
	InitHelper(ShadowQualityDropdown);
	InitHelper(GlobalIlluminationDropdown);
	InitHelper(ReflectionQualityDropdown);
	InitHelper(TextureQualityDropdown);
	InitHelper(EffectsQualityDropdown);
	InitHelper(PostProcessingDropdown);
	InitHelper(FoliageQualityDropdown);
	InitHelper(ShadingQualityDropdown);
}

UWidget* UCustomQualitySettingsWidget::GetFirstFocusableChild() const
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

UWidget* UCustomQualitySettingsWidget::GetLastFocusableChild() const
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
