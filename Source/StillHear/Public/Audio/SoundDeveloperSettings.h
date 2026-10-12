// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SoundDeveloperSettings.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FSoundLevels
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float Master = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float Music = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float Ambience = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float SFX = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float UI = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound Levels", meta=(ClampMin="0", ClampMax="1"))
	float Voice = 1.f;
};


UCLASS(Config=GameSounds, DefaultConfig, meta=(DisplayName="Sound Settings"))
class STILLHEAR_API USoundDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_Master;

	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_Music;

	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_Ambience;
	
	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_SFX;

	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_UI;
	
	UPROPERTY(Config, EditAnywhere, Category="Sound Classes")
	TSoftObjectPtr<class USoundClass> SC_Voice;
	
	UPROPERTY(Config, EditAnywhere, Category="Submix")
	TSoftObjectPtr<class USoundSubmix> SM_World;
	
	UPROPERTY(Config, EditAnywhere, Category="SoundMix")
	TSoftObjectPtr<class USoundMix> SMX_RunTimeSettings;
	
	UPROPERTY(Config, EditAnywhere, Category="Sound Classes|Gameplay")
	TSoftObjectPtr<class USoundClass> SC_SFX_Gameplay; 


	UPROPERTY(Config, EditAnywhere, Category="Audio States")
	TSoftObjectPtr<class UAudioStateConfig> AudioStateConfig;

	UPROPERTY(Config, EditAnywhere, Category="Default Levels")
	FSoundLevels DefaultLevels;

	// Time it takes to fade the music volume in/out when ducking/unducking
	UPROPERTY(Config, EditAnywhere, Category="Ducking", meta=(ClampMin="0"))
	float DuckFadeTime = 0.3f;
	
	// Fraction of the normal music volume used while the game is paused
	UPROPERTY(Config, EditAnywhere, Category="Ducking|Pause", meta=(ClampMin="0", ClampMax="1"))
	float PauseDuckMultiplier = 0.3f;
	
	FName GetCategoryName() const override { return FName("StillHear"); }
#if WITH_EDITOR
	FText GetSectionText() const override { return NSLOCTEXT("StillHear", "GameSound", "Game Sound"); }
#endif
	
};
