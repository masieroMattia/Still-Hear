#include "UI/Indicator/IndicatorLayer.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/Indicator/IndicatorSubsystem.h"
#include "UI/Indicator/IndicatorWidgetBase.h"
#include "UI/Indicator/IndicatorDescriptor.h"

#pragma region METHODS
void UIndicatorLayer::NativeConstruct()
{
	Super::NativeConstruct();
    
    if (GetWorld())
        CachedIndicatorSubsystem = GetWorld()->GetSubsystem<UIndicatorSubsystem>();
}

void UIndicatorLayer::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (!IndicatorCanvas || !CachedIndicatorSubsystem) 
        return;

    const APlayerController* PC = GetOwningPlayer();
    if (!PC) 
        return;

    const TArray<TObjectPtr<UIndicatorDescriptor>>& Indicators = CachedIndicatorSubsystem->GetIndicators();

    // Find widgets whose descriptors are no longer active and remove them
    TArray<UIndicatorDescriptor*> DescriptorsToRemove;
    for (const auto& Pair : SpawnedWidgets)
    {
        UIndicatorDescriptor* TrackedDescriptor = Pair.Key;
        if (!TrackedDescriptor || !Indicators.Contains(TrackedDescriptor))
        {
            if (UUserWidget* WidgetToRemove = Pair.Value)
                WidgetToRemove->RemoveFromParent();
            
            DescriptorsToRemove.Add(TrackedDescriptor);
        }
    }
    for (UIndicatorDescriptor* ToRemove : DescriptorsToRemove)
        SpawnedWidgets.Remove(ToRemove);

    // Use the canvas panel's own geometry as the authoritative coordinate space
    const FGeometry CanvasGeo  = IndicatorCanvas->GetCachedGeometry();
    const FVector2D CanvasSize = CanvasGeo.GetLocalSize();
    const FVector2D CanvasCenter = CanvasSize * 0.5f;
    const float ViewportScale  = UWidgetLayoutLibrary::GetViewportScale(this);

    // Safe zone — every edge is inset by ScreenEdgeMargin
    const float SafeMinX = ScreenEdgeMargin;
    const float SafeMaxX = CanvasSize.X - ScreenEdgeMargin;
    const float SafeMinY = ScreenEdgeMargin;
    const float SafeMaxY = CanvasSize.Y - ScreenEdgeMargin;

    // Process all currently active indicators
    for (UIndicatorDescriptor* Descriptor : Indicators)
    {
        if (!Descriptor || !Descriptor->TargetActor) 
            continue;

        // Spawn the widget if it doesn't exist yet
        if (!SpawnedWidgets.Contains(Descriptor) && Descriptor->IndicatorWidgetClass)
        {
            UIndicatorWidgetBase* NewWidget = CreateWidget<UIndicatorWidgetBase>(this, Descriptor->IndicatorWidgetClass); 
            if (NewWidget)
            {
                NewWidget->Descriptor = Descriptor;
                IndicatorCanvas->AddChild(NewWidget);
                SpawnedWidgets.Add(Descriptor, NewWidget);
            }
        }

        UUserWidget* ActiveWidget = SpawnedWidgets[Descriptor];
        if (!ActiveWidget) continue;

        UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ActiveWidget->Slot);
        if (!CanvasSlot) continue;

        CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        CanvasSlot->SetAutoSize(true);

        const FVector WorldLocation = Descriptor->TargetActor->GetActorLocation() + Descriptor->WorldOffset;

        // ProjectWorldLocationToScreen → physical viewport pixels
        // Dividing by ViewportScale converts to Slate logical units, which match
        // the canvas panel's local coordinate space for a fullscreen widget at origin
        FVector2D ScreenPosition;
        const bool bProjectedInFrontOfCamera = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);
        const FVector2D LocalPosition = ScreenPosition / ViewportScale;

        // On-screen: in front of camera AND within the full canvas bounds
        const bool bIsWithinViewport = bProjectedInFrontOfCamera
            && LocalPosition.X >= 0.0f && LocalPosition.X <= CanvasSize.X
            && LocalPosition.Y >= 0.0f && LocalPosition.Y <= CanvasSize.Y;

        bool bIsOccluded = false;
        if (bIsWithinViewport && Descriptor->bCheckOcclusion)
        {
            FVector CameraLocation;
            FRotator CameraRotation;
            PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

            FHitResult HitResult;
            FCollisionQueryParams OcclusionParams(SCENE_QUERY_STAT(IndicatorOcclusion), true);
            OcclusionParams.AddIgnoredActor(Descriptor->TargetActor);
            if (PC->GetPawn())
                OcclusionParams.AddIgnoredActor(PC->GetPawn());

            bIsOccluded = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, WorldLocation, ECC_Visibility, OcclusionParams);
        }

        const bool bIsEffectivelyOnScreen = bIsWithinViewport && !bIsOccluded;

        if (bIsEffectivelyOnScreen)
        {
            if (Descriptor->bShowOnlyOffScreen)
            {
                ActiveWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
            else
            {
                // Follow the target exactly, clamped to the margin-inset safe zone
                const FVector2D Clamped(FMath::Clamp(LocalPosition.X, SafeMinX, SafeMaxX), FMath::Clamp(LocalPosition.Y, SafeMinY, SafeMaxY));
                CanvasSlot->SetPosition(Clamped);
                ActiveWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            }
        }
        else if (bIsOccluded || Descriptor->bClampToScreen)
        {
            // Build a 2D direction from canvas center toward the target
            FVector2D Dir;

            if (bProjectedInFrontOfCamera)
            {
                // Target in front of camera but off-screen — projected position is valid
                Dir = LocalPosition - CanvasCenter;
            }
            else
            {
                // Target behind the camera — use camera right/up dot products for direction
                FVector CameraLocation;
                FRotator CameraRotation;
                PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

                const FVector CameraToTarget = (WorldLocation - CameraLocation).GetSafeNormal();
                const FMatrix CamMatrix = FRotationMatrix(CameraRotation);
                const float RightDot = FVector::DotProduct(CameraToTarget, CamMatrix.GetScaledAxis(EAxis::Y));
                const float UpDot    = FVector::DotProduct(CameraToTarget, CamMatrix.GetScaledAxis(EAxis::Z));

                // Scale large enough to guarantee clamping always snaps to the edge
                Dir = FVector2D(RightDot, -UpDot) * (CanvasCenter + FVector2D(1.0f, 1.0f)) * 2.0f;
            }

            // Ray–rectangle intersection: find where Dir exits the margin-inset safe zone
            FVector2D FinalPosition = CanvasCenter; // fallback
            if (!Dir.IsNearlyZero())
            {
                // Half-extents of the safe rectangle (centred on CanvasCenter)
                const float HalfW = SafeMaxX - CanvasCenter.X;  // = CanvasSize.X/2 - ScreenEdgeMargin
                const float HalfH = SafeMaxY - CanvasCenter.Y;  // = CanvasSize.Y/2 - ScreenEdgeMargin

                const float AbsX = FMath::Abs(Dir.X);
                const float AbsY = FMath::Abs(Dir.Y);
                const float ScaleX = (AbsX > KINDA_SMALL_NUMBER) ? (HalfW / AbsX) : FLT_MAX;
                const float ScaleY = (AbsY > KINDA_SMALL_NUMBER) ? (HalfH / AbsY) : FLT_MAX;
                
                // Prevents moving a position already inside the safe zone
                const float EdgeScale = FMath::Min(FMath::Min(ScaleX, ScaleY), 1.0f);
                FinalPosition = CanvasCenter + Dir * EdgeScale;
            }

            CanvasSlot->SetPosition(FinalPosition);
            ActiveWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            ActiveWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
#pragma endregion