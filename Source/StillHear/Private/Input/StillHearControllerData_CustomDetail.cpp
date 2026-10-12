#include "Input/StillHearControllerData_CustomDetail.h"
#if WITH_EDITOR
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Input/StillHearControllerData.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FStillHearControllerData_CustomDetail"

TSharedRef<IDetailCustomization> FStillHearControllerData_CustomDetail::MakeInstance()
{
	return MakeShareable(new FStillHearControllerData_CustomDetail);
}

void FStillHearControllerData_CustomDetail::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Get the object being customized
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() > 0)
	{
		CustomizedObject = Cast<UStillHearControllerData>(ObjectsBeingCustomized[0].Get());
	}

	IDetailCategoryBuilder& CustomCategory = DetailLayout.EditCategory("Gamepad");
	CustomCategory.AddCustomRow(LOCTEXT("UpdateGamepadName", "Update Gamepad Name"))
	.ValueContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("UpdateGamepadNameButton", "Update Gamepad Name"))
		.OnClicked(FOnClicked::CreateSP(this, &FStillHearControllerData_CustomDetail::OnUpdateGamepadNameClicked))
	];
}

FReply FStillHearControllerData_CustomDetail::OnUpdateGamepadNameClicked()
{
	if (CustomizedObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling UpdateGamepadName on %s"), *CustomizedObject->GetName());
		CustomizedObject->UpdateGamepadName();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CustomizedObject is null"));
	}
 
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
#endif