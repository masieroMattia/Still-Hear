// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/GameSettings.h"

#include "CommonAnimatedSwitcher.h"
#include "UI/Widgets/Controls/ControlsSettingsWidget.h"

void UGameSettings::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	ControlsSettingsWidget->OnBindingsPageOpened.AddUniqueDynamic(this, &ThisClass::DeactivateBackInputAction);
	ControlsSettingsWidget->OnBindingsPageClosed.AddUniqueDynamic(this, &ThisClass::ActivateBackInputAction);
}

void UGameSettings::NativeOnDeactivated()
{
	ControlsSettingsWidget->OnBindingsPageOpened.RemoveDynamic(this, &ThisClass::DeactivateBackInputAction);
	ControlsSettingsWidget->OnBindingsPageClosed.RemoveDynamic(this, &ThisClass::ActivateBackInputAction);

	Super::NativeOnDeactivated();
}

void UGameSettings::HandleBackActionBinding()
{	
	if (bInterceptBackInputAction)
	{
		if (ControlsSettingsWidget->IsBindingsPageOpen())
		{
			ControlsSettingsWidget->CloseBindingsPage();
		}
		return;
	}
	
	Super::HandleBackActionBinding();
}

void UGameSettings::HandleSwitcherTransitionFinished(bool bIsTransitioning)
{
	Super::HandleSwitcherTransitionFinished(bIsTransitioning);

	if (!bIsTransitioning && ContentSwitcher)
	{
		UControlsSettingsWidget* ActivePage = Cast<UControlsSettingsWidget>(ContentSwitcher->GetActiveWidget());
		
		if (!ActivePage)
		{
			ControlsSettingsWidget->CloseBindingsPage();
		}
	}
}

void UGameSettings::DeactivateBackInputAction()
{
	bInterceptBackInputAction = true;
}

void UGameSettings::ActivateBackInputAction()
{
	bInterceptBackInputAction = false;
}

