#include "Subsystems/TimeManagementSubsystem.h"

#include "Kismet/GameplayStatics.h"

void UTimeManagementSubsystem::Deinitialize()
{
    ResetTimeDilation();
    Super::Deinitialize();
}

void UTimeManagementSubsystem::PlayTimeCurve(UCurveFloat* Curve)
{
    if (!Curve)
        return;

    ActiveTimeCurve = Curve;
    ElapsedCurveTime = 0.0f;

    // Remove existing ticker if a slo-mo is already playing
    if (TimeTickerHandle.IsValid())
        FTSTicker::GetCoreTicker().RemoveTicker(TimeTickerHandle);

    // Register the ticker to fire every frame using unscaled real-world time
    TimeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UTimeManagementSubsystem::TickTimeCurve));
}

bool UTimeManagementSubsystem::TickTimeCurve(const float DeltaTime)
{
    if (!ActiveTimeCurve || !GetWorld())
    {
        ResetTimeDilation();
        return false;
    }

    // Pause the curve progression if a hit stop is currently active
    if (bIsHitStopActive)
        return true;

    ElapsedCurveTime += DeltaTime;

    float MinTime, MaxTime;
    ActiveTimeCurve->GetTimeRange(MinTime, MaxTime);

    if (ElapsedCurveTime >= MaxTime)
    {
        ResetTimeDilation();
        return false; 
    }

    const float CurrentDilation = ActiveTimeCurve->GetFloatValue(ElapsedCurveTime);
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), CurrentDilation);

    return true; 
}

void UTimeManagementSubsystem::TriggerHitStop(const float Duration)
{
    if (!GetWorld() || bIsHitStopActive)
        return;

    bIsHitStopActive = true;
    
    // Save the current dilation so can be restored later
    PreviousTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

    // Freeze the game
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.001f);

    // Set an unscaled timer (ignores time dilation) to resume the game
    GetWorld()->GetTimerManager().SetTimer(
        HitStopTimerHandle,
        this,
        &UTimeManagementSubsystem::EndHitStop,
        Duration,
        false,
        -1.0f
    );
}

void UTimeManagementSubsystem::EndHitStop()
{
    bIsHitStopActive = false;
    
    // Restore the time dilation to what it was before the hit stop
    if (GetWorld())
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PreviousTimeDilation);
}

void UTimeManagementSubsystem::ResetTimeDilation()
{
    if (TimeTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TimeTickerHandle);
        TimeTickerHandle.Reset();
    }
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HitStopTimerHandle);
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
    }
        
    ActiveTimeCurve = nullptr;
    bIsHitStopActive = false;
}