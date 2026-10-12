#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "PC_MainMenu.generated.h"

class UCommonActivatableWidget;

UCLASS()
class STILLHEAR_API APC_MainMenu : public APlayerController
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCommonActivatableWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FGameplayTag MainMenuLayerTag;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;

	// Function called to push the widget to the layout
	UFUNCTION()
	void PushMainMenuWidget();
#pragma endregion
};