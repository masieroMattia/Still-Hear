#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PSOLoadingWidget.generated.h"

class UCommonTextBlock;
struct FPSOMessageData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPSOCompilationFinished);

UCLASS()
class STILLHEAR_API UPSOLoadingWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
#pragma region UPROPERTIES
protected:
	// Reference to the DataTable containing the messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSO Loading|Data")
	UDataTable* LoadingMessagesTable;

	// Time in seconds between message changes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSO Loading|Data")
	float MessageChangeInterval = 3.0f;
	
	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* PercentageText;

	UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* MessageText;
#pragma endregion
	
#pragma region EVENTS
public:
	UPROPERTY(BlueprintAssignable, Category = "PSO Loading")
	FOnPSOCompilationFinished OnCompilationFinishedDelegate;
#pragma endregion
	
#pragma region VARIABLES
private:
	int32 InitialPSOCount = 0;
	int32 DotCount = 0;
	int32 CurrentRowIndex = 0;

	float DotTimer = 0.0f;
	float MessageTimer = 0.0f;
    
	TArray<FPSOMessageData*> CachedMessages;
#pragma endregion

#pragma region METHODS
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	void UpdateCurrentMessage(float DeltaTime);
	FString GetDisplayString();
	void UpdateUI(int32 Percentage, const FString& LoadingText) const;
#pragma endregion
};