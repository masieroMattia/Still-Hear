#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CustomHUD.generated.h"

class UPrimaryGameLayout;

UCLASS()
class STILLHEAR_API ACustomHUD : public AHUD
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// The visual Blueprint class of the Primary Game Layout to spawn
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPrimaryGameLayout> PrimaryLayoutClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPrimaryGameLayout> SpawnedLayout;
#pragma endregion
	
#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

};
