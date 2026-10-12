// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Controls/Keyboard/BindingsListWidget.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "UI/Widgets/Controls/Keyboard/BindingRowWidget.h"

void UBindingsListWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBindingsListWidget::InitializeRows()
{
	for (int i = 0; i < RowWidgets.Num(); i++)
	{
		RowWidgets[i]->OnKeyIsNotDefault.RemoveDynamic(this, &ThisClass::FoundNotDefaultKey);
	}
	RowWidgets.Empty();
	
	for (const auto& Row : ListContainer->GetAllChildren())
	{
		UBindingRowWidget* RowWidget = Cast<UBindingRowWidget>(Row);
		if (RowWidget)
		{
			RowWidgets.Add(RowWidget);
		}
	}
	
	for (int i = 0; i < RowWidgets.Num(); i++)
	{
		RowWidgets[i]->OnKeyIsNotDefault.AddUniqueDynamic(this, &ThisClass::FoundNotDefaultKey);
		RowWidgets[i]->SetInitialGlyph();
	}
}

void UBindingsListWidget::FoundNotDefaultKey()
{
	OnFoundNotDefaultKey.Broadcast();
}
