#include "UI/Widgets/HoldProgressWidget.h"

#include "Components/Image.h"
#include "EnhancedInputSubsystems.h"
#include "UI/Widgets/InputActionWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/Application/SlateApplication.h"

// Listens for ANY input just to reveal the "Hold to skip" prompt without consuming the input
class FAnyInputListener : public IInputProcessor
{
public:
	TWeakObjectPtr<UHoldProgressWidget> Widget;
	TSet<FKey> PressedKeys;

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		PressedKeys.Add(InKeyEvent.GetKey());
		if (Widget.IsValid())
			Widget->NotifyAnyInput();
		return false; 
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		PressedKeys.Remove(InKeyEvent.GetKey());
		return false;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		PressedKeys.Add(MouseEvent.GetEffectingButton());
		if (Widget.IsValid())
			Widget->NotifyAnyInput();
		return false; 
	}

	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		PressedKeys.Remove(MouseEvent.GetEffectingButton());
		return false;
	}

	bool IsKeyDown(const FKey& Key) const
	{
		return PressedKeys.Contains(Key);
	}
};

void UHoldProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ProgressMaterial && ProgressRing)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(ProgressMaterial, this);
		ProgressRing->SetBrushFromMaterial(DynamicMaterial);
	}

	if (InputActionIcon && HoldAction)
		InputActionIcon->SetupInputAction(HoldAction);

	ApplyProgress(0.0f);
	SetRenderOpacity(0.0f);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UHoldProgressWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	TSharedRef<FAnyInputListener> Listener = MakeShared<FAnyInputListener>();
	Listener->Widget = this;
	AnyKeyListener = Listener;
	FSlateApplication::Get().RegisterInputPreProcessor(AnyKeyListener);

	MappedKeysCache.Empty();
	if (HoldAction)
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			if (const ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (const UEnhancedInputLocalPlayerSubsystem* InputSys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					MappedKeysCache = InputSys->QueryKeysMappedToAction(HoldAction);
				}
			}
		}
	}
}

void UHoldProgressWidget::NativeOnDeactivated()
{
	if (AnyKeyListener)
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(AnyKeyListener);
		AnyKeyListener.Reset();
	}

	GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	ResetProgress();
	Super::NativeOnDeactivated();
}

void UHoldProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	bool bCurrentlyHolding = false;

	// Infallible physical key check
	// Bypasses Common UI entirely to prevent focus routing issues
	if (HoldAction && AnyKeyListener.IsValid())
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			if (MappedKeysCache.IsEmpty())
			{
				if (const ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (const UEnhancedInputLocalPlayerSubsystem* InputSys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						MappedKeysCache = InputSys->QueryKeysMappedToAction(HoldAction);
					}
				}
			}

			if (MappedKeysCache.Num() > 0)
			{
				TSharedPtr<FAnyInputListener> Listener = StaticCastSharedPtr<FAnyInputListener>(AnyKeyListener);
				for (const FKey& Key : MappedKeysCache)
				{
					if (Listener->IsKeyDown(Key) || PC->IsInputKeyDown(Key))
					{
						bCurrentlyHolding = true;
						break;
					}
				}
			}
		}
	}

	// Synchronize logical state based on physical keys
	if (bCurrentlyHolding && !bIsHolding)
	{
		OnHoldPressed();
	}
	else if (!bCurrentlyHolding && bIsHolding)
	{
		OnHoldReleased();
	}

	// Advance or decay progress if not yet completed
	if (!bCompleted)
	{
		if (bIsHolding)
		{
			CurrentProgress = FMath::Clamp(CurrentProgress + (InDeltaTime / HoldDuration), 0.0f, 1.0f);
			ApplyProgress(CurrentProgress);

			if (CurrentProgress >= 1.0f)
			{
				OnHoldCompleted();
			}
		}
		else if (CurrentProgress > 0.0f)
		{
			// Gradually decrease progress when not holding
			CurrentProgress = FMath::Clamp(CurrentProgress - (InDeltaTime / HoldDuration), 0.0f, 1.0f);
			ApplyProgress(CurrentProgress);
		}
	}
}

void UHoldProgressWidget::OnHoldPressed()
{
	// Reset progress only if it was fully completed before
	if (bCompleted)
	{
		bCompleted = false;
		CurrentProgress = 0.0f;
	}
	
	bIsHolding = true;

	GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	SetRenderOpacity(1.0f);
	
	if (InputActionIcon)
		InputActionIcon->PressAction();
}

void UHoldProgressWidget::OnHoldReleased()
{
	bIsHolding = false;

	GetWorld()->GetTimerManager().SetTimer(
		AutoHideTimer, this, &UHoldProgressWidget::OnAutoHideTimerExpired, AutoHideDelay, false);

	if (InputActionIcon)
		InputActionIcon->ReleaseAction();
}

void UHoldProgressWidget::OnHoldCompleted()
{
	if (bCompleted)
		return;

	bCompleted = true;
	CurrentProgress = 1.0f;
	ApplyProgress(1.0f);

	if (InputActionIcon)
		InputActionIcon->ReleaseAction();

	OnHoldComplete.Broadcast();
}

void UHoldProgressWidget::ApplyProgress(const float Progress) const
{
	if (DynamicMaterial)
		DynamicMaterial->SetScalarParameterValue(ProgressParameterName, Progress);
}

void UHoldProgressWidget::NotifyAnyInput()
{
	SetRenderOpacity(1.0f);

	if (!bIsHolding)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoHideTimer, this, &UHoldProgressWidget::OnAutoHideTimerExpired, AutoHideDelay, false);
	}
}

void UHoldProgressWidget::OnAutoHideTimerExpired()
{
	if (!bIsHolding)
		SetRenderOpacity(0.0f);
}

void UHoldProgressWidget::ResetProgress()
{
	CurrentProgress = 0.0f;
	bCompleted = false;
	ApplyProgress(0.0f);
}
