#include "UI/Widgets/DLSSSettingsWidget.h"

#include "DLSSLibrary.h"
#include "HAL/IConsoleManager.h"
#include "Blueprint/WidgetTree.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "UI/Elements/DropdownBase.h"
#include "Engine/GameViewportClient.h"
#include "UI/Widgets/SettingsRowBase.h"
#include "GameFramework/GameUserSettings.h"

#pragma region UFUNCTIONS
void UDLSSSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitializeDropdowns();
}

void UDLSSSettingsWidget::ApplyDLSSSettings()
{
	ApplySuperResolution();
	ApplyFrameGeneration();
	ApplyReflex();
}
#pragma endregion

#pragma region METHODS
void UDLSSSettingsWidget::ApplySuperResolution() const
{
	if (!SuperResolutionDropdown)
		return;

	const int32 SelectedIndex = SuperResolutionDropdown->GetSelectedIndex();

	// Helper to set r.ScreenPercentage preserving the CVar's existing priority
	auto SetScreenPercentage = [](const float Value)
	{
		static IConsoleVariable* CVarSP = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
		if (CVarSP)
		{
			const EConsoleVariableFlags Priority = static_cast<EConsoleVariableFlags>(CVarSP->GetFlags() & ECVF_SetByMask);
			CVarSP->Set(Value, Priority != 0 ? Priority : ECVF_SetByGameSetting);
		}
	};

	// ── Index 0: OFF ──────────────────────────────────────────────────────────
	if (SelectedIndex == 0)
	{
		UDLSSLibrary::EnableDLSS(false);
		SetScreenPercentage(100.0f);
		return;
	}

	// Map UI index → DLSS quality mode
	UDLSSMode TargetMode = GetDLSSModeFromIndex(SelectedIndex);

	// ── Auto: resolve to the best concrete mode for current resolution ────────
	if (TargetMode == UDLSSMode::Auto)
	{
		TargetMode = UDLSSLibrary::GetDefaultDLSSMode();
		// If GetDefaultDLSSMode returns Off or Auto (shouldn't happen), fall back to Quality
		if (TargetMode == UDLSSMode::Off || TargetMode == UDLSSMode::Auto)
			TargetMode = UDLSSMode::Quality;
	}

	// ── DLAA: screen percentage = 100 + EnableDLSS ────────────────────────────
	if (TargetMode == UDLSSMode::DLAA)
	{
		SetScreenPercentage(100.0f);
		UDLSSLibrary::EnableDLSS(true);
		return;
	}

	// ── Quality / Performance / etc. ─────────────────────────────────────────
	// Determine current viewport resolution for mode information query
	FVector2D Resolution(1920.f, 1080.f);
	if (GEngine && GEngine->GameUserSettings)
	{
		const FIntPoint Res = GEngine->GameUserSettings->GetScreenResolution();
		Resolution = FVector2D(Res.X, Res.Y);
	}

	bool bIsSupported = false;
	float OptimalSP = 100.f; bool bFixed = false; float MinSP = 50.f, MaxSP = 100.f, Sharpness = 0.f;
	UDLSSLibrary::GetDLSSModeInformation(TargetMode, Resolution, bIsSupported, OptimalSP, bFixed, MinSP, MaxSP, Sharpness);

	if (bIsSupported && OptimalSP > 0.f)
	{
		SetScreenPercentage(OptimalSP);
	}
	else
	{
		// Fallback: use 67% (equivalent to Quality mode)
		SetScreenPercentage(66.7f);
	}

	UDLSSLibrary::EnableDLSS(true);
}

void UDLSSSettingsWidget::ApplyFrameGeneration() const
{
	if (!FrameGenerationDropdown)
		return;

	EStreamlineDLSSGMode TargetMode = EStreamlineDLSSGMode::Off;
	if (UStreamlineLibraryDLSSG::IsDLSSGSupported())
	{
		TargetMode = GetDLSSGModeFromIndex(FrameGenerationDropdown->GetSelectedIndex());
	}
	UStreamlineLibraryDLSSG::SetDLSSGMode(TargetMode);
}

void UDLSSSettingsWidget::ApplyReflex()
{
	if (!ReflexDropdown)
		return;

	const EStreamlineReflexMode TargetMode = GetReflexModeFromIndex(ReflexDropdown->GetSelectedIndex());
	UStreamlineLibraryReflex::SetReflexMode(TargetMode);
}

void UDLSSSettingsWidget::SetSavedIndices(const int32 SRIndex, const int32 FGIndex, const int32 RflxIndex) const
{
	if (SuperResolutionDropdown) 
		SuperResolutionDropdown->SetSelectedIndex(SRIndex);
	
	if (FrameGenerationDropdown) 
		FrameGenerationDropdown->SetSelectedIndex(FGIndex);
	
	if (ReflexDropdown) 
		ReflexDropdown->SetSelectedIndex(RflxIndex);
}

int32 UDLSSSettingsWidget::GetSuperResolutionIndex() const
{
	return SuperResolutionDropdown ? SuperResolutionDropdown->GetSelectedIndex() : 0;
}

int32 UDLSSSettingsWidget::GetFrameGenerationIndex() const
{
	return FrameGenerationDropdown ? FrameGenerationDropdown->GetSelectedIndex() : 0;
}

int32 UDLSSSettingsWidget::GetReflexIndex() const
{
	return ReflexDropdown ? ReflexDropdown->GetSelectedIndex() : 0;
}

void UDLSSSettingsWidget::InitializeDropdowns()
{
	if (SuperResolutionDropdown)
	{
		const TArray SROptions = {
			FText::FromString("OFF"),
			FText::FromString("AUTO"),
			FText::FromString("DLAA"),
			FText::FromString("ULTRA QUALITY"),
			FText::FromString("QUALITY"),
			FText::FromString("BALANCED"),
			FText::FromString("PERFORMANCE"),
			FText::FromString("ULTRA PERFORMANCE")
		};
		
		SuperResolutionDropdown->SetOptions(SROptions);
		SuperResolutionDropdown->SetSelectedIndex(0);
		SuperResolutionDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UDLSSSettingsWidget::HandleSubSettingChanged);
	}
	if (FrameGenerationDropdown)
	{
		if (!UStreamlineLibraryDLSSG::IsDLSSGSupported())
		{
			FrameGenerationDropdown->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			const TArray FGOptions = {
				FText::FromString("OFF"),
				FText::FromString("AUTO"),
				FText::FromString("DYNAMIC"),
				FText::FromString("2X"),
				FText::FromString("3X"),
				FText::FromString("4X"),
				FText::FromString("5X"),
				FText::FromString("6X")
			};
			
			FrameGenerationDropdown->SetOptions(FGOptions);
			FrameGenerationDropdown->SetSelectedIndex(0);
			FrameGenerationDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UDLSSSettingsWidget::HandleSubSettingChanged);
		}
	}
	if (ReflexDropdown)
	{
		const TArray ReflexOptions = {
			FText::FromString("OFF"),
			FText::FromString("ON"),
			FText::FromString("ON + BOOST")
		};
		
		ReflexDropdown->SetOptions(ReflexOptions);
		ReflexDropdown->SetSelectedIndex(0);
		ReflexDropdown->OnSelectionChanged.AddUniqueDynamic(this, &UDLSSSettingsWidget::HandleSubSettingChanged);
	}
}

void UDLSSSettingsWidget::HandleSubSettingChanged(const int32 SelectedIndex, FText SelectedText)
{
	OnSettingsChanged.Broadcast();
}
#pragma endregion

#pragma region HELPERS
UDLSSMode UDLSSSettingsWidget::GetDLSSModeFromIndex(const int32 Index)
{
	switch (Index)
	{
		case 1:  return UDLSSMode::Auto;
		case 2:  return UDLSSMode::DLAA;
		case 3:  return UDLSSMode::UltraQuality;
		case 4:  return UDLSSMode::Quality;
		case 5:  return UDLSSMode::Balanced;
		case 6:  return UDLSSMode::Performance;
		case 7:  return UDLSSMode::UltraPerformance;
		default: return UDLSSMode::Quality;
	}
}

EStreamlineDLSSGMode UDLSSSettingsWidget::GetDLSSGModeFromIndex(const int32 Index)
{
	switch (Index)
	{
		case 0:  return EStreamlineDLSSGMode::Off;
		case 1:  return EStreamlineDLSSGMode::Auto;
		case 2:  return EStreamlineDLSSGMode::OnDynamic;
		case 3:  return EStreamlineDLSSGMode::On2X;
		case 4:  return EStreamlineDLSSGMode::On3X;
		case 5:  return EStreamlineDLSSGMode::On4X;
		case 6:  return EStreamlineDLSSGMode::On5X;
		case 7:  return EStreamlineDLSSGMode::On6X;
		default: return EStreamlineDLSSGMode::Off;
	}
}

EStreamlineReflexMode UDLSSSettingsWidget::GetReflexModeFromIndex(const int32 Index)
{
	switch (Index)
	{
		case 0:  return EStreamlineReflexMode::Off;
		case 1:  return EStreamlineReflexMode::Enabled;
		case 2:  return EStreamlineReflexMode::Boost;
		default: return EStreamlineReflexMode::Off;
	}
}

UWidget* UDLSSSettingsWidget::GetFirstFocusableChild() const
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

UWidget* UDLSSSettingsWidget::GetLastFocusableChild() const
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