#pragma once

#include "CoreMinimal.h"

class FStillHearModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if WITH_EDITOR
private:
	// Registers extensions for the Level Editor
	void RegisterLevelEditorExtensions();

	// Callback for the Level Editor to retrieve the menu extender
	TSharedRef<FExtender> GetLevelViewportExtender(const TSharedRef<FUICommandList>& CommandList, const TArray<AActor*>& SelectedActors);

	// Populates the context menu with custom tools and submenus
	void OnFillLevelViewportContextMenu(FMenuBuilder& MenuBuilder) const;

	// Handle to the registered extender, used for unregistration
	FDelegateHandle LevelEditorMenuExtenderHandle;
#endif
};

