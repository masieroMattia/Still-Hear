#pragma once
 
#include "CoreMinimal.h"
#if WITH_EDITOR
#include "IDetailCustomization.h"
 
class FStillHearControllerData_CustomDetail : public IDetailCustomization
{

#pragma region VARIABLES
private:
	// Store a pointer to the object being customized
	TObjectPtr<class UStillHearControllerData> CustomizedObject;
#pragma endregion

#pragma region METHODS
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
 
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
 
private:
	FReply OnUpdateGamepadNameClicked();
#pragma endregion
 
};
#endif
