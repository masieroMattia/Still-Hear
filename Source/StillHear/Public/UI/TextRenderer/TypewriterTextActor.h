#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "TypewriterTextActor.generated.h"

class UTextRenderComponent;
class UNiagaraSystem;

// DataTable row used to store localized typewriter texts
USTRUCT(BlueprintType)
struct FTypewriterTextRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TextRow;
};

// World-space text actor that reveals characters one by one with a Niagara effect
UCLASS()
class STILLHEAR_API ATypewriterTextActor : public AActor
{
	GENERATED_BODY()

#pragma region UPROPERTIES
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	// ── Text Source ──────────────────────────────────────────────────────────
	// DataTable with row type FTypewriterTextRow (optional — overrides FullText)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Text")
	TObjectPtr<UDataTable> TextDataTable;

	// Row name to read from the DataTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Text", meta = (EditCondition = "TextDataTable != nullptr", EditConditionHides))
	FName TextRowName;

	// Fallback text used when no DataTable is set. Use \n for manual line breaks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Text", meta = (EditCondition = "TextDataTable == nullptr", EditConditionHides))
	FString FullText = TEXT("Insert text here");

	// ── Layout ───────────────────────────────────────────────────────────────
	// Auto word-wrap: max characters per line (0 = disabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Text", meta = (ClampMin = "0"))
	int32 MaxCharsPerLine = 0;

	// Characters revealed per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|Text", meta = (ClampMin = "1.0"))
	float CharactersPerSecond = 12.0f;

	//Fraction of the text that is currently visible (0 = none, 1 = all)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp, Category = "Typewriter|Text", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float RevealProgress = 0.0f;

	// ── FX ───────────────────────────────────────────────────────────────────
	// Niagara system spawned at each new letter position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|FX")
	TObjectPtr<UNiagaraSystem> LetterFX;

	// Sound played for each revealed character (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|FX")
	TObjectPtr<USoundBase> LetterSound;

	// Volume of the per-letter sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|FX", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float LetterSoundVolume = 0.3f;

	// Approximate width of one character in world units (used to offset FX)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter|FX", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float CharacterWorldWidth = 8.0f;
#pragma endregion

#pragma region VARIABLES
private:
	int32 CurrentVisibleCount = 0;
	FTimerHandle RevealTimerHandle;

	// FullText after DataTable lookup, \n conversion, and word-wrap
	FString ProcessedText;
#pragma endregion

#pragma region CONSTRUCTOR
public:
	ATypewriterTextActor();
#pragma endregion

#pragma region METHODS
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// Returns the active text: DataTable row if set, otherwise FullText
	FString ResolveSourceText() const;

	void ApplyRevealProgress(float NewProgress);
	void BuildProcessedText();
	FVector GetCharacterWorldPosition(int32 CharIndex) const;
	void PlayLetterFX(const FVector& WorldPosition) const;
#pragma endregion

#pragma region UFUNCTIONS
public:
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void StartReveal();

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void StopReveal();

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void CompleteReveal();

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void ResetReveal();

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void SetRevealProgress(float NewProgress);

	// Reload text from the DataTable (call at runtime if the row changes)
	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void RefreshTextFromDataTable();
#pragma endregion
};

