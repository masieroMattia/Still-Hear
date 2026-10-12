#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UISubsystem.generated.h"

class UCommonActivatableWidget;
class UPrimaryGameLayout;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUILayoutReady);

UCLASS()
class STILLHEAR_API UUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
private:
	// Reference to the active root layout on the screen
	UPROPERTY(Transient)
	TObjectPtr<UPrimaryGameLayout> RootLayout;
#pragma endregion
	
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnUILayoutReady OnUILayoutReady;
#pragma endregion
	
#pragma region METHODS
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
#pragma endregion

#pragma region UFUNCTIONS
public:
	// Sets the primary layout
	UFUNCTION(BlueprintCallable, Category = "UI|Layout")
	void SetPrimaryLayout(UPrimaryGameLayout* InLayout);

	// Returns the current Root Layout
	UFUNCTION(BlueprintPure, Category = "UI|Layout")
	UPrimaryGameLayout* GetPrimaryLayout() const;

	// Pushes a widget to the specified layer using the Root Layout
	UFUNCTION(BlueprintCallable, meta = (Categories = "UI.Layer"), Category = "UI|Widgets")
	UCommonActivatableWidget* PushWidgetToLayer(const FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, const bool bClearLayer = false, const bool bPauseGame = false);

	// Clears all widgets from the specified layer using the Root Layout
	UFUNCTION(BlueprintCallable, meta = (Categories = "UI.Layer"), Category = "UI|Widgets")
	void ClearLayer(const FGameplayTag LayerTag);
#pragma endregion
};
