#include "UI/Elements/SaveSlotButtonBase.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/SaveGameObject.h"

TWeakObjectPtr<USaveSlotButtonBase> USaveSlotButtonBase::FocusedSlotButton = nullptr;
int32 USaveSlotButtonBase::SelectedSlotIndex = -1;

#pragma region METHODS
void USaveSlotButtonBase::NativeConstruct()
{
   Super::NativeConstruct();
   bFocusOnHover = false;
   OnClicked().AddUObject(this, &USaveSlotButtonBase::HandleSlotClicked);
   RefreshSlotInfo();
}

void USaveSlotButtonBase::NativeOnCurrentTextStyleChanged()
{
   Super::NativeOnCurrentTextStyleChanged();

   if (DateText)
      DateText->SetStyle(GetCurrentTextStyleClass());

   if (LevelNameText)
      LevelNameText->SetStyle(GetCurrentTextStyleClass());
}

FReply USaveSlotButtonBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
   FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
   
   USaveSlotButtonBase* OldFocused = FocusedSlotButton.Get();
   FocusedSlotButton = this;
   SelectedSlotIndex = SlotIndex;
   
   if (OldFocused && OldFocused != this)
      OldFocused->UpdateFocusIndicator();
   
   UpdateFocusIndicator();
   
   return Reply;
}

void USaveSlotButtonBase::UpdateFocusIndicator()
{
   if (FocusIndicator)
      FocusIndicator->SetVisibility((FocusedSlotButton == this) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void USaveSlotButtonBase::NativeOnHovered()
{
   Super::NativeOnHovered();

   // With gamepad, CommonUI moves hover state (not Slate focus) between buttons
   // Track selection via hover so the indicator follows the gamepad cursor
   if (LastInputType != ECommonInputType::MouseAndKeyboard)
   {
      USaveSlotButtonBase* OldFocused = FocusedSlotButton.Get();
      FocusedSlotButton = this;
      SelectedSlotIndex = SlotIndex;
      
      if (OldFocused && OldFocused != this)
         OldFocused->UpdateFocusIndicator();
      
      UpdateFocusIndicator();
   }
}

void USaveSlotButtonBase::NativeDestruct()
{
   if (FocusedSlotButton == this)
   {
      FocusedSlotButton = nullptr;
      SelectedSlotIndex = -1;
   }
   Super::NativeDestruct();
}

void USaveSlotButtonBase::RefreshSlotInfo()
{
   const FString TargetSlotName = FString::Printf(TEXT("Save_Slot%d"), SlotIndex);

   if (UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0))
   {
      if (const USaveGameObject* Save = Cast<USaveGameObject>(UGameplayStatics::LoadGameFromSlot(TargetSlotName, 0)))
      {
         if (DateText)
         {
            DateText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            DateText->SetText(FText::AsDateTime(Save->SaveDate, EDateTimeStyle::Short, EDateTimeStyle::Short));
         }

         FText CheckpointName;
         TSoftObjectPtr<UTexture2D> CheckpointImage;
         bool bHasCheckpointData = false;

         if (const UGameInstance* GI = GetGameInstance())
         {
            if (USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>())
               bHasCheckpointData = SaveSub->GetCheckpointDisplayData(Save->CheckpointId, CheckpointName, CheckpointImage);
         }

         if (LevelNameText)
         {
            LevelNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            LevelNameText->SetText(bHasCheckpointData
               ? CheckpointName
               : FText::FromString(FPackageName::GetShortName(Save->LastLevelName)));
         }

         if (LevelImage)
         {
            if (bHasCheckpointData && !CheckpointImage.IsNull())
            {
               LevelImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
               LevelImage->SetBrushFromTexture(CheckpointImage.LoadSynchronous());
            }
            else
               LevelImage->SetVisibility(ESlateVisibility::Collapsed);
         }

         ButtonText = FText::FromString(FString::Printf(TEXT("SLOT %d"), SlotIndex));
         RefreshButtonContent();
         return;
      }
   }

   if (DateText)
      DateText->SetVisibility(ESlateVisibility::Collapsed);

   if (LevelImage)
      LevelImage->SetVisibility(ESlateVisibility::Collapsed);

   if (LevelNameText)
   {
      LevelNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
      LevelNameText->SetText(EmptySlotText);
   }
}

void USaveSlotButtonBase::ApplyDisplay(const FText& Name, const TSoftObjectPtr<UTexture2D>& Image, const FText& InButtonText)
{
   if (DateText)
      DateText->SetVisibility(ESlateVisibility::Collapsed);

   if (LevelNameText)
   {
      LevelNameText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
      LevelNameText->SetText(Name);
   }

   if (LevelImage)
   {
      if (!Image.IsNull())
      {
         LevelImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
         LevelImage->SetBrushFromTexture(Image.LoadSynchronous());
      }
      else
         LevelImage->SetVisibility(ESlateVisibility::Collapsed);
   }

   if (!InButtonText.IsEmpty())
   {
      ButtonText = InButtonText;
      RefreshButtonContent();
   }
}

void USaveSlotButtonBase::HandleSlotClicked()
{
   USaveSlotButtonBase* OldFocused = FocusedSlotButton.Get();
   SelectedSlotIndex = SlotIndex;
   FocusedSlotButton = this;
   
   if (OldFocused && OldFocused != this)
      OldFocused->UpdateFocusIndicator();
   
   SetFocus();
}

USaveSlotButtonBase* USaveSlotButtonBase::GetFocusedSlotButton()
{
   return FocusedSlotButton.Get();
}

int32 USaveSlotButtonBase::GetSelectedSlotIndex()
{
   return SelectedSlotIndex;
}
#pragma endregion