#include "StillHear.h"

#include "Modules/ModuleManager.h"
#include "Input/StillHearControllerData_CustomDetail.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"
#include "PropertyEditorModule.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#endif

IMPLEMENT_PRIMARY_GAME_MODULE( FStillHearModule, StillHear, "StillHear" );

void FStillHearModule::StartupModule()
{
	// Register custom detail layouts for data assets/classes
#if WITH_EDITOR
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("StillHearControllerData", FOnGetDetailCustomizationInstance::CreateStatic(&FStillHearControllerData_CustomDetail::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	// Initialize editor-specific tools and menu extensions
	RegisterLevelEditorExtensions();
#endif
}

void FStillHearModule::ShutdownModule()
{
	// Cleanup registered customizations
#if WITH_EDITOR
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("StillHearControllerData");
	}

	// Unregister level editor extensions to avoid memory leaks or crashes when the module is reloaded
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([this](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Delegate) 
		{
			return Delegate.GetHandle() == LevelEditorMenuExtenderHandle;
		});
	}
#endif
}

#if WITH_EDITOR
void FStillHearModule::RegisterLevelEditorExtensions()
{
	// Access the Level Editor module to add custom functionality to the level viewport
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	// Add a new extender delegate to the global list of viewport context menu extenders
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	
	MenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateLambda(
		[this](const TSharedRef<FUICommandList>& CommandList, const TArray<AActor*>& SelectedActors)
		{
			return GetLevelViewportExtender(CommandList, SelectedActors);
		}
	));
	
	// Store the handle so we can safely remove it later
	LevelEditorMenuExtenderHandle = MenuExtenders.Last().GetHandle();
}

TSharedRef<FExtender> FStillHearModule::GetLevelViewportExtender(const TSharedRef<FUICommandList>& CommandList, const TArray<AActor*>& SelectedActors)
{
	TSharedRef<FExtender> Extender = MakeShareable(new FExtender());

	// Hook into the 'LevelViewportAttach' point, which is where attachment options usually appear
	Extender->AddMenuExtension(
		"LevelViewportAttach",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FStillHearModule::OnFillLevelViewportContextMenu)
	);

	return Extender;
}

void FStillHearModule::OnFillLevelViewportContextMenu(FMenuBuilder& MenuBuilder) const
{
	// Create a dedicated section for our custom tools
	MenuBuilder.BeginSection("StillHearUtils", FText::FromString("StillHear Utils"));
	{
		// Add a submenu that lists all PlayerStart actors in the current level
		MenuBuilder.AddSubMenu(
			FText::FromString("Move Player Start Here"),
			FText::FromString("Choose a Player Start to move to this location"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& SubMenuBuilder)
			{
				if (!GEditor) return;

				// Retrieve the active world in the editor
				UWorld* World = GEditor->GetEditorWorldContext().World();
				if (!World) return;

				bool bFoundAny = false;
				// Iterate through all PlayerStart actors in the current level
				for (TActorIterator<APlayerStart> It(World); It; ++It)
				{
					APlayerStart* PS = *It;
					if (PS)
					{
						bFoundAny = true;
						FText ActorName = FText::FromString(PS->GetActorLabel());
						
						// Add a menu entry for each found PlayerStart
						SubMenuBuilder.AddMenuEntry(
							ActorName,
							FText::Format(FText::FromString("Move {0} to cursor location"), ActorName),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateLambda([PS]()
							{
								if (PS && GEditor)
								{
									// Mark for Undo/Redo
									PS->Modify();
									
									// Move to the right-click location, offset slightly upward to avoid spawning inside the ground
									PS->SetActorLocation(GEditor->ClickLocation + FVector(0, 0, 95.0f));
									
									// Align rotation with the current editor camera (ignoring pitch/roll)
									if (GCurrentLevelEditingViewportClient)
									{
										FRotator NewRot = GCurrentLevelEditingViewportClient->GetViewRotation();
										NewRot.Pitch = 0.0f;
										NewRot.Roll = 0.0f;
										PS->SetActorRotation(NewRot);
									}
									
									// Select the moved actor for immediate feedback
									GEditor->SelectNone(true, true);
									GEditor->SelectActor(PS, true, true);
								}
							}))
						);
					}
				}

				// Fallback if no PlayerStarts exist in the level
				if (!bFoundAny)
				{
					SubMenuBuilder.AddMenuEntry(
						FText::FromString("No Player Start found in level"),
						FText::GetEmpty(),
						FSlateIcon(),
						FUIAction(),
						NAME_None,
						EUserInterfaceActionType::None
					);
				}
			})
		);
	}
	MenuBuilder.EndSection();
}
#endif
