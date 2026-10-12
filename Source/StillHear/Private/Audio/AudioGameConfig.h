
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioGameConfig.generated.h"


UCLASS(BlueprintType)
class STILLHEAR_API UAudioGameConfig : public UDataAsset
{
	GENERATED_BODY()
	
public: 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundClasses")
	USoundClass* SC_Master = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundClasses")
	USoundClass* SC_Music = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundClasses")
	USoundClass* SC_SFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundClasses")
	USoundClass* SC_UI = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundMix")
	USoundMix* SMX_Gameplay = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundMix")
	USoundMix* SMX_Menu = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SoundMix")
	USoundMix* SMX_RunTimeSettings = nullptr;
	

};
