#include "UI/Widgets/GraphicsDebugWidget.h"

#include "DLSSLibrary.h"
#include "HAL/IConsoleManager.h"
#include "Engine/GameInstance.h"
#include "Components/TextBlock.h"
#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "GameFramework/GameUserSettings.h"

#pragma region UFUNCTIONS
void UGraphicsDebugWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDebugText();
}

void UGraphicsDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeSinceLastRefresh += InDeltaTime;
	if (TimeSinceLastRefresh >= RefreshInterval)
	{
		TimeSinceLastRefresh = 0.f;
		RefreshDebugText();
	}
}
#pragma endregion

#pragma region METHODS
void UGraphicsDebugWidget::RefreshDebugText()
{
	if (!DebugText) return;

#pragma region READ CVARS
	auto GetCVarInt = [](const TCHAR* Name) -> int32
	{
		if (const IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(Name))
			return CV->GetInt();
		return -1;
	};

	auto GetCVarFloat = [](const TCHAR* Name) -> float
	{
		if (const IConsoleVariable* CV = IConsoleManager::Get().FindConsoleVariable(Name))
			return CV->GetFloat();
		return -1.f;
	};

	const int32  AAMethod       = GetCVarInt(TEXT("r.AntiAliasingMethod"));
	const float  ScreenPct      = GetCVarFloat(TEXT("r.ScreenPercentage"));
	const int32  DLSSEnabled    = GetCVarInt(TEXT("r.NGX.DLSS.Enable"));
	const int32  TAA_Upscaler   = GetCVarInt(TEXT("r.TemporalAA.Upscaler"));
	const int32  TAA_Upsampling = GetCVarInt(TEXT("r.TemporalAA.Upsampling"));
	const int32  FSREnabled     = GetCVarInt(TEXT("r.FidelityFX.FSR.Enabled"));
	const int32  FSRQuality     = GetCVarInt(TEXT("r.FidelityFX.FSR.QualityMode"));
	const int32  FIEnabled      = GetCVarInt(TEXT("r.FidelityFX.FI.Enabled"));
	const int32  DLSSGMode      = GetCVarInt(TEXT("r.Streamline.DLSSG.Enable"));

	const bool bDLSSActive = UDLSSLibrary::IsDLSSEnabled();

	FIntPoint Resolution(0, 0);
	if (GEngine && GEngine->GameUserSettings)
		Resolution = GEngine->GameUserSettings->GetScreenResolution();
#pragma endregion

#pragma region READ SAVE
	FString SavedUpscaler = TEXT("N/A");
	int32 SavedDLSS_SR = -1, SavedFSR_SR = -1;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
		{
			if (const USettingsSaveGame* S = SS->GetSaveSettings())
			{
				SavedUpscaler = S->ActiveUpscaler;
				SavedDLSS_SR  = S->DLSS_SuperResolutionIndex;
				SavedFSR_SR   = S->FSR_SuperResolutionIndex;
			}
		}
	}
#pragma endregion

#pragma region RESOLVE ACTUAL UPSCALER
	FString ActualUpscaler;
	if (bDLSSActive)
		ActualUpscaler = TEXT("DLSS-SR");
	else if (FSREnabled > 0)
		ActualUpscaler = TEXT("FSR");
	else if (AAMethod == 4)
		ActualUpscaler = TEXT("TSR");
	else if (AAMethod == 2 || AAMethod == 0)
		ActualUpscaler = TEXT("TAA/None");
	else
		ActualUpscaler = FString::Printf(TEXT("AA=%d"), AAMethod);
#pragma endregion

#pragma region BOX BUILDER
	// Each row: ║ + InnerW chars + ║\n  →  total visible = InnerW + 2
	constexpr int32 InnerW = 46;   // number of chars between the two ║
	constexpr int32 LabelW = 16;   // label column width
	constexpr int32 ValueW = InnerW - 2 - LabelW - 3;  // 25  (2 indent + 3 sep)

	// Pad or truncate S to exactly Width chars
	auto PadTo = [](FString S, int32 Width) -> FString
	{
		if (S.Len() > Width) return S.Left(Width);
		while (S.Len() < Width) S.AppendChar(TEXT(' '));
		return S;
	};

	// Full content row:  "  {label,-16} │ {value,-25}"
	auto Row = [&PadTo, LabelW, ValueW](const FString& Label, const FString& Value) -> FString
	{
		const FString Content = TEXT("  ") + PadTo(Label, LabelW) + TEXT(" \u2502 ") + PadTo(Value, ValueW);
		return TEXT("\u2551") + Content + TEXT("\u2551\n");
	};

	// Centered header row
	auto Header = [InnerW](const FString& Title) -> FString
	{
		const int32 LP = (InnerW - Title.Len()) / 2;
		const int32 RP = InnerW - Title.Len() - LP;
		FString Content;
		for (int32 i = 0; i < LP; ++i) Content.AppendChar(TEXT(' '));
		Content += Title;
		for (int32 i = 0; i < RP; ++i) Content.AppendChar(TEXT(' '));
		return TEXT("\u2551") + Content + TEXT("\u2551\n");
	};

	// Horizontal border/divider strings
	FString HorzBar;
	for (int32 i = 0; i < InnerW; ++i) HorzBar.AppendChar(TEXT('\u2550'));

	const FString TopBorder = TEXT("\u2554") + HorzBar + TEXT("\u2557\n");
	const FString Divider   = TEXT("\u2560") + HorzBar + TEXT("\u2563\n");
	const FString BotBorder = TEXT("\u255a") + HorzBar + TEXT("\u255d");
#pragma endregion

#pragma region BUILD OUTPUT
	FString Out;
	Out += TopBorder;
	Out += Header(TEXT("GRAPHICS DEBUG OVERLAY"));
	Out += Divider;

	Out += Row(TEXT("RENDERING NOW"), ActualUpscaler);
	Out += Row(TEXT("SAVED"),
		FString::Printf(TEXT("%-5s  DLSS_SR=%-2d  FSR_SR=%-2d"), *SavedUpscaler, SavedDLSS_SR, SavedFSR_SR));

	Out += Divider;

	Out += Row(TEXT("AA Method"),
		FString::Printf(TEXT("%d  (%s)%s"), AAMethod, *GetAAMethodName(AAMethod),
			(bDLSSActive || FSREnabled > 0) ? TEXT("  <overridden>") : TEXT("")));
	Out += Row(TEXT("ScreenPercentage"),
		FString::Printf(TEXT("%.2f %%"), ScreenPct));
	Out += Row(TEXT("Resolution"),
		FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));

	Out += Divider;

	Out += Row(TEXT("DLSS Enable"),
		FString::Printf(TEXT("%d  (%s)"), DLSSEnabled, DLSSEnabled > 0 ? TEXT("ON") : TEXT("OFF")));
	Out += Row(TEXT("TAA Upscaler"),   FString::FromInt(TAA_Upscaler));
	Out += Row(TEXT("TAA Upsampling"), FString::FromInt(TAA_Upsampling));
	Out += Row(TEXT("DLSS-FG Mode"),   FString::FromInt(DLSSGMode));

	Out += Divider;

	Out += Row(TEXT("FSR Enabled"),
		FString::Printf(TEXT("%d  (%s)"), FSREnabled, FSREnabled > 0 ? TEXT("ON") : TEXT("OFF")));
	Out += Row(TEXT("FSR Quality"),    FString::FromInt(FSRQuality));
	Out += Row(TEXT("FSR FI Enabled"), FString::FromInt(FIEnabled));

	Out += BotBorder;

	DebugText->SetText(FText::FromString(Out));
#pragma endregion
}

#pragma endregion

#pragma region HELPERS
FString UGraphicsDebugWidget::GetAAMethodName(int32 Method)
{
	switch (Method)
	{
		case 0: return TEXT("None");
		case 1: return TEXT("FXAA");
		case 2: return TEXT("TAA");
		case 3: return TEXT("MSAA");
		case 4: return TEXT("TSR");
		default: return FString::Printf(TEXT("Unknown(%d)"), Method);
	}
}
#pragma endregion