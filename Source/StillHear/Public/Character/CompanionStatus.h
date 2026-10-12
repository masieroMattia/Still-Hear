#pragma once

// Companion Status Enum
UENUM(BlueprintType)
enum class ECompanionStatus : uint8
{
	Joy		UMETA(DisplayName = "Joy"),
	Anger	UMETA(DisplayName = "Anger"),
	Fear	UMETA(DisplayName = "Fear")
};
