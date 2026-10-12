#include "Input/InputSubsystem.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "CommonInputSubsystem.h"
#include "InputMappingContext.h"
#include "StillHearGameInstance.h"
#include "Input/BindingData.h"
#include "Input/InputDeviceType.h"
#include "Input/MappingContextList.h"
#include "Input/StillHearControllerData.h"
#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/SettingsSaveGame.h"
#include "InputModifiers.h"
#include "Input/ControllerCategory.h"
#include "UI/Widgets/Controls/Keyboard/KeyboardMoveDirection.h"

UInputSubsystem::UInputSubsystem()
{
	
}

void UInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	USaveSubsystem* SaveSubsystem = Collection.InitializeDependency<USaveSubsystem>();

	UStillHearGameInstance* GameInstance = CastChecked<UStillHearGameInstance>(GetGameInstance());

	UMappingContextList* MappingContextList = GameInstance->GetMappingContextListDataAsset().LoadSynchronous();

	if (!MappingContextList)
	{
#if WITH_EDITOR
		if (GEngine)
			GEngine->AddOnScreenDebugMessage( -1, 10.f, FColor::Red, "MappingContextList is not set in GameInstance");
#endif
		return;
	}

	AllMappingContexts = MappingContextList->MappingContexts;

	CacheDefaultBindings();
	
	const USettingsSaveGame* Settings = SaveSubsystem->LoadSettings() ? SaveSubsystem->GetSaveSettings() : nullptr;

	if (IsValid(Settings) && Settings->Bindings.Num() != 0)
	{
#if WITH_EDITOR
		if (GEngine)
			GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Yellow, "Esistono settings salvati, applico le binding salvate");
#endif
		ApplySavedBindings(Settings);
	}
	
}

void UInputSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UInputSubsystem::CacheDefaultBindings()
{
	for (const auto& MappingContext : AllMappingContexts)
	{
		//GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Yellow, "Cache Binding Iniziato");
		for (const auto& Mapping : MappingContext->GetMappings())
		{
			FBindingData BindingData;
			BindingData.InputAction = const_cast<UInputAction*>(Mapping.Action.Get());
			BindingData.DefaultBoundKey = Mapping.Key;
			BindingData.CurrentBoundKey = Mapping.Key;
			BindingData.DeviceType = Mapping.Key.IsGamepadKey() ? EInputDeviceType::Controller : EInputDeviceType::KeyboardMouse;
			CurrentBindings.Add(BindingData);
		}
		//GEngine->AddOnScreenDebugMessage( -1, 5.f, FColor::Yellow, "Cache Binding Finito: " + FString::FromInt(CurrentBindings.Num()));
	}
}

void UInputSubsystem::ApplySavedBindings(const class USettingsSaveGame* Settings)
{
	CurrentBindings = Settings->Bindings;

	struct FPendingRemap
	{
		TArray<UInputModifier*> Modifiers;
		UInputMappingContext* Context = nullptr;
		UInputAction* Action = nullptr;
		FKey FromKey; // default, to remove
		FKey ToKey;   // current saved, to add
	};

	TArray<FPendingRemap> Pending;

	// Phase 1: capture while the context is still to default
	for (const auto& Binding : CurrentBindings)
	{
		if (Binding.CurrentBoundKey == Binding.DefaultBoundKey)
			continue;

		for (const auto& MappingContext : AllMappingContexts)
		{
			bool bFound = false;
			for (const auto& Mapping : MappingContext->GetMappings())
			{
				if (Mapping.Action == Binding.InputAction && Mapping.Key == Binding.DefaultBoundKey)
				{
					FPendingRemap Remap;
					Remap.Modifiers = Mapping.Modifiers;
					Remap.Context = MappingContext;
					Remap.Action = Binding.InputAction;
					Remap.FromKey = Binding.DefaultBoundKey;
					Remap.ToKey = Binding.CurrentBoundKey;
					Pending.Add(Remap);
					bFound = true;
					break;
				}
			}
			if (bFound) break;
		}
	}

	// Phase 2: first remove all the default mappings
	for (const FPendingRemap& Remap : Pending)
		Remap.Context->UnmapKey(Remap.Action, Remap.FromKey);

	// Phase 3: add saved bindings with correct modifiers
	for (const FPendingRemap& Remap : Pending)
	{
		FEnhancedActionKeyMapping& NewMapping = Remap.Context->MapKey(Remap.Action, Remap.ToKey);
		NewMapping.Modifiers = Remap.Modifiers;
	}
}

bool UInputSubsystem::HasNegateModifier(const FEnhancedActionKeyMapping& Mapping)
{
	for (const UInputModifier* Modifier : Mapping.Modifiers)
	{
		if (Modifier && Modifier->IsA<UInputModifierNegate>())
		{
			return true;
		}
	}

	return false;
}

bool UInputSubsystem::HasSwizzleModifier(const FEnhancedActionKeyMapping& Mapping)
{
	for (const UInputModifier* Modifier : Mapping.Modifiers)
	{
		if (Modifier && Modifier->IsA<UInputModifierSwizzleAxis>())
		{
			return true;
		}
	}

	return false;
}

void UInputSubsystem::ApplyDefaultBindings()
{
	struct FPendingRemap
	{
		int32 BindingIndex = INDEX_NONE;
		TArray<UInputModifier*> Modifiers;
		UInputMappingContext* Context = nullptr;
		UInputAction* Action = nullptr;
		FKey FromKey; // current, to remove
		FKey ToKey;   // default, to add
	};

	TArray<FPendingRemap> Pending;

	// Phase 1: capture while the context is coherent
	for (int32 i = 0; i < CurrentBindings.Num(); i++)
	{
		FBindingData& Binding = CurrentBindings[i];

		if (Binding.CurrentBoundKey == Binding.DefaultBoundKey)
			continue; // already default, nothing to do

		for (const auto& MappingContext : AllMappingContexts)
		{
			bool bFound = false;
			for (const auto& Mapping : MappingContext->GetMappings())
			{
				if (Mapping.Action == Binding.InputAction && Mapping.Key == Binding.CurrentBoundKey)
				{
					FPendingRemap Remap;
					Remap.BindingIndex = i;
					Remap.Modifiers = Mapping.Modifiers;
					Remap.Context = MappingContext;
					Remap.Action = Binding.InputAction;
					Remap.FromKey = Binding.CurrentBoundKey;
					Remap.ToKey = Binding.DefaultBoundKey;
					Pending.Add(Remap);
					bFound = true;
					break;
				}
			}
			if (bFound) break;
		}
	}

	// Phase 2: first remove all current mappings
	for (const FPendingRemap& Remap : Pending)
		Remap.Context->UnmapKey(Remap.Action, Remap.FromKey);

	// Phase 3: add new default mappings while restoring the modifiers
	for (const FPendingRemap& Remap : Pending)
	{
		FEnhancedActionKeyMapping& NewMapping = Remap.Context->MapKey(Remap.Action, Remap.ToKey);
		NewMapping.Modifiers = Remap.Modifiers;
		CurrentBindings[Remap.BindingIndex].CurrentBoundKey = Remap.ToKey;
	}
}

void UInputSubsystem::AddToKeysToRebind(const FBindingData& BindingData)
{
	RebindUtilityArray.Add(BindingData);
}

void UInputSubsystem::AddToKeysToRebindArray(TArray<FBindingData>& Bindings)
{
	RebindUtilityArray.Append(MoveTemp(Bindings));
}

void UInputSubsystem::ClearKeysToRebind()
{
	if (RebindUtilityArray.Num() > 0)
		RebindUtilityArray.Empty();
}

void UInputSubsystem::RebindKeys()
{	
	struct FPendingRemap
	{
		FBindingData Binding;
		TArray<UInputModifier*> Modifiers;
		UInputMappingContext* Context = nullptr;
		int32 CurrentBindingIndex = INDEX_NONE;
	};

	TArray<FPendingRemap> Pending;

	// Phase 1: get the index and modifier while the context is coherent
	for (const auto& Binding : RebindUtilityArray)
	{
		for (const auto& MappingContext : AllMappingContexts)
		{
			bool bFound = false;
			for (const auto& Mapping : MappingContext->GetMappings())
			{
				if (Mapping.Action == Binding.InputAction && Mapping.Key == Binding.DefaultBoundKey)
				{
					FPendingRemap Remap;
					Remap.Binding = Binding;
					Remap.Modifiers = Mapping.Modifiers;
					Remap.Context = MappingContext;

					for (int32 i = 0; i < CurrentBindings.Num(); i++)
					{
						if (CurrentBindings[i].InputAction == Binding.InputAction &&
							CurrentBindings[i].CurrentBoundKey == Binding.DefaultBoundKey)
						{
							Remap.CurrentBindingIndex = i;
							break;
						}
					}

					Pending.Add(Remap);
					bFound = true;
					break;
				}
			}
			if (bFound)
				break;
		}
	}

	// Phase 2: first remove all the old mappings
	for (const FPendingRemap& Remap : Pending)
	{
		Remap.Context->UnmapKey(Remap.Binding.InputAction, Remap.Binding.DefaultBoundKey);
	}

	// Phase 3: add new mappings while keeping the modifiers
	for (const FPendingRemap& Remap : Pending)
	{
		FEnhancedActionKeyMapping& NewMapping = Remap.Context->MapKey(Remap.Binding.InputAction, Remap.Binding.CurrentBoundKey);
		NewMapping.Modifiers = Remap.Modifiers;

		if (Remap.CurrentBindingIndex != INDEX_NONE)
		{
			CurrentBindings[Remap.CurrentBindingIndex].CurrentBoundKey = Remap.Binding.CurrentBoundKey;
		}
	}

	ClearKeysToRebind();
}

FKey UInputSubsystem::GetCurrentKeyForAction(class UInputAction* InputAction, const EInputDeviceType DeviceType)
{
	FKey ResultKey;

	for (const auto& Binding : CurrentBindings)
	{
		if (Binding.InputAction == InputAction && Binding.DeviceType == DeviceType)
		{
			ResultKey = Binding.CurrentBoundKey;
			break;
		}
	}
	
	return ResultKey;
}

FKey UInputSubsystem::GetDefaultKeyForAction(class UInputAction* InputAction, const FKey CurrentKey)
{
	FKey ResultKey;

	for (const auto& Binding : CurrentBindings)
	{
		if (Binding.InputAction == InputAction && Binding.CurrentBoundKey == CurrentKey)
		{
			ResultKey = Binding.DefaultBoundKey;
			break;
		}
	}
	
	return ResultKey;
}

FKey UInputSubsystem::GetCurrentKeyForMoveDirection(class UInputAction* InputAction, const EKeyboardMoveDirection Direction)
{
	for (const auto& MappingContext : AllMappingContexts)
	{
		for (const auto& Mapping : MappingContext->GetMappings())
		{
			if (Mapping.Action != InputAction || Mapping.Key.IsGamepadKey())
				continue;

			const bool bNegate = HasNegateModifier(Mapping);
			const bool bSwizzle = HasSwizzleModifier(Mapping);

			bool bMatch = false;
			switch (Direction)
			{
			case EKeyboardMoveDirection::Forward:  bMatch =  bSwizzle && !bNegate; break; // only Swizzle
			case EKeyboardMoveDirection::Left:     bMatch = !bSwizzle &&  bNegate; break; // only Negate
			case EKeyboardMoveDirection::Right:    bMatch = !bSwizzle && !bNegate; break; // none
			case EKeyboardMoveDirection::Backward: bMatch =  bSwizzle &&  bNegate; break; // both
			}

			if (bMatch)
				return Mapping.Key;
		}
	}

	return FKey(); // invalid: no direction found
}

FSlateBrush UInputSubsystem::GetBrushFromKey(const FKey Key, const EInputDeviceType DeviceType) const
{
	const UCommonInputBaseControllerData* DeviceData = nullptr;

	if (DeviceType == EInputDeviceType::KeyboardMouse)
		DeviceData = CastChecked<UStillHearGameInstance>(GetGameInstance())->GetKeyboardData()->GetDefaultObject<UCommonInputBaseControllerData>();
	else if (DeviceType == EInputDeviceType::Controller)
	{
		if (CurrentControllerType == EControllerCategory::PlayStation)
			DeviceData = CastChecked<UStillHearGameInstance>(GetGameInstance())->GetPlaystationData()->GetDefaultObject<UCommonInputBaseControllerData>();
		else if (CurrentControllerType == EControllerCategory::Xbox)
			DeviceData = CastChecked<UStillHearGameInstance>(GetGameInstance())->GetXboxData()->GetDefaultObject<UCommonInputBaseControllerData>();
	}

	FSlateBrush Brush;
	bool bFound = DeviceData->TryGetInputBrush(
		Brush,
		Key
	);

	return Brush;
}

bool UInputSubsystem::IsInputActionSetToDefault(UInputAction* InputAction, const FKey& Key)
{
	bool Result = false;
	
	for (const auto& Binding : CurrentBindings)
	{
		if (Binding.InputAction == InputAction && Binding.CurrentBoundKey == Key)
		{
			if (Binding.DefaultBoundKey == Binding.CurrentBoundKey)
				Result = true;
			else
				Result = false;

			break;
		}
	}

	return Result;
}

void UInputSubsystem::SetControllerInputType(const FName& ControllerName)
{
	UWorld* World = GetWorld();

	if (!World)
		return;
	
	ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();

	if (!LocalPlayer)
		return;
	
	UCommonInputSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>();
	InputSubsystem->SetGamepadInputType(FName(ControllerName));
}

void UInputSubsystem::SaveBindings()
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	USettingsSaveGame* Settings = SaveSubsystem->GetSaveSettings();
	Settings->Bindings = CurrentBindings;
	SaveSubsystem->SaveSettingsAsync();
}

void UInputSubsystem::DeleteBindings()
{
	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	USettingsSaveGame* Settings = SaveSubsystem->GetSaveSettings();
	Settings->Bindings = {};
	SaveSubsystem->SaveSettingsAsync();
}

void UInputSubsystem::ResetSavedBindingsToDefault()
{
	ApplyDefaultBindings();
	SaveBindings();
}
