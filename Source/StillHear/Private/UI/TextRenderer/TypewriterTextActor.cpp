#include "UI/TextRenderer/TypewriterTextActor.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/TextRenderComponent.h"

#pragma region CONSTRUCTOR
ATypewriterTextActor::ATypewriterTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	TextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComponent"));
	TextRenderComponent->SetHorizontalAlignment(EHTA_Center);
	TextRenderComponent->SetVerticalAlignment(EVRTA_TextCenter);
	TextRenderComponent->SetText(FText::GetEmpty());
	RootComponent = TextRenderComponent;

	BuildProcessedText();
}
#pragma endregion

#pragma region METHODS
void ATypewriterTextActor::BeginPlay()
{
	Super::BeginPlay();

	BuildProcessedText();
	TextRenderComponent->SetText(FText::GetEmpty());
	CurrentVisibleCount = 0;
}

void ATypewriterTextActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const int32 Expected = FMath::RoundToInt(RevealProgress * ProcessedText.Len());
	if (Expected != CurrentVisibleCount)
		ApplyRevealProgress(RevealProgress);
}

#if WITH_EDITOR
void ATypewriterTextActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	BuildProcessedText();

	const int32 TargetCount = FMath::RoundToInt(RevealProgress * ProcessedText.Len());
	const FString Visible = ProcessedText.Left(FMath::Clamp(TargetCount, 0, ProcessedText.Len()));
	TextRenderComponent->SetText(FText::FromString(Visible));
}
#endif

FString ATypewriterTextActor::ResolveSourceText() const
{
	if (TextDataTable && !TextRowName.IsNone())
	{
		if (const FTypewriterTextRow* Row = TextDataTable->FindRow<FTypewriterTextRow>(TextRowName, TEXT("TypewriterTextActor")))
			return Row->TextRow;
	}
	return FullText;
}

void ATypewriterTextActor::BuildProcessedText()
{
	// Resolve source (DataTable or FullText)
	ProcessedText = ResolveSourceText();

	// Convert literal \n → real newline
	ProcessedText = ProcessedText.Replace(TEXT("\\n"), TEXT("\n"));

	// Word-wrap at MaxCharsPerLine (0 = disabled)
	if (MaxCharsPerLine <= 0)
		return;

	TArray<FString> Words;
	ProcessedText.ParseIntoArray(Words, TEXT(" "), true);

	FString Wrapped;
	int32 LineLen = 0;

	for (const FString& Word : Words)
	{
		if (LineLen > 0)
		{
			if (LineLen + 1 + Word.Len() > MaxCharsPerLine)
			{
				Wrapped += TEXT("\n");
				LineLen = 0;
			}
			else
			{
				Wrapped += TEXT(" ");
				LineLen += 1;
			}
		}

		Wrapped += Word;

		int32 LastNL = INDEX_NONE;
		Word.FindLastChar(TEXT('\n'), LastNL);
		LineLen = (LastNL != INDEX_NONE) ? (Word.Len() - LastNL - 1) : (LineLen + Word.Len());
	}

	ProcessedText = Wrapped;
}

void ATypewriterTextActor::ApplyRevealProgress(const float NewProgress)
{
	const int32 TargetCount = FMath::RoundToInt(NewProgress * ProcessedText.Len());
	const int32 Clamped = FMath::Clamp(TargetCount, 0, ProcessedText.Len());

	if (Clamped == CurrentVisibleCount)
		return;

	if (Clamped > CurrentVisibleCount)
	{
		for (int32 i = CurrentVisibleCount; i < Clamped; i++)
		{
			if (ProcessedText[i] != TEXT('\n'))
				PlayLetterFX(GetCharacterWorldPosition(i));
		}
	}

	CurrentVisibleCount = Clamped;
	TextRenderComponent->SetText(FText::FromString(ProcessedText.Left(CurrentVisibleCount)));
}

FVector ATypewriterTextActor::GetCharacterWorldPosition(const int32 CharIndex) const
{
	const float TotalWidth = ProcessedText.Len() * CharacterWorldWidth;
	const float OffsetX = (CharIndex * CharacterWorldWidth) - (TotalWidth * 0.5f) + (CharacterWorldWidth * 0.5f);
	return GetActorLocation() + GetActorRightVector() * OffsetX;
}

void ATypewriterTextActor::PlayLetterFX(const FVector& WorldPosition) const
{
	if (LetterFX)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LetterFX, WorldPosition, GetActorRotation());

	if (LetterSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), LetterSound, WorldPosition, LetterSoundVolume);
}
#pragma endregion

#pragma region UFUNCTIONS
void ATypewriterTextActor::StartReveal()
{
	if (GetWorldTimerManager().IsTimerActive(RevealTimerHandle))
		return;

	const float Interval = 1.0f / FMath::Max(CharactersPerSecond, 1.0f);
	GetWorldTimerManager().SetTimer(RevealTimerHandle, [this]()
	{
		const float Step = 1.0f / FMath::Max(static_cast<float>(ProcessedText.Len()), 1.0f);
		SetRevealProgress(RevealProgress + Step);
		if (RevealProgress >= 1.0f)
			GetWorldTimerManager().ClearTimer(RevealTimerHandle);
	}, Interval, true);
}

void ATypewriterTextActor::StopReveal()
{
	GetWorldTimerManager().ClearTimer(RevealTimerHandle);
}

void ATypewriterTextActor::CompleteReveal()
{
	StopReveal();
	SetRevealProgress(1.0f);
}

void ATypewriterTextActor::ResetReveal()
{
	StopReveal();
	SetRevealProgress(0.0f);
}

void ATypewriterTextActor::SetRevealProgress(const float NewProgress)
{
	RevealProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);
	ApplyRevealProgress(RevealProgress);
}

void ATypewriterTextActor::RefreshTextFromDataTable()
{
	BuildProcessedText();
	ApplyRevealProgress(RevealProgress);
}
#pragma endregion