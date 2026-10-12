#pragma once

UENUM(BlueprintType)
enum class E_AIType : uint8
{
	MANTIS		UMETA(DisplayName = "Mantis"),
	WORM		UMETA(DisplayName = "Worm")
};

UENUM(BlueprintType)
enum class E_AISpeedType : uint8
{
	UNSET		UMETA(DisplayName = "Unset"),
	WALK		UMETA(DisplayName = "Walk"),
	RUN			UMETA(DisplayName = "Run")
};

UENUM(BlueprintType)
enum class E_AITag : uint8
{
	UNAWARE				UMETA(DisplayName = "Unaware"),
	STUNNED             UMETA(DisplayName = "Stunned"),
	SUSPICIOUS			UMETA(DisplayName = "Suspicious"),
	ALERTED				UMETA(DisplayName = "Alerted"),
	HUNTING				UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class E_AISense : uint8
{
	NONE		UMETA(DisplayName = "None"),
	SIGHT		UMETA(DisplayName = "Sight"),
	HEARING		UMETA(DisplayName = "Hearing"),
	TOUCH		UMETA(DisplayName = "Touch"),
};

UENUM(BlueprintType)
enum class E_AIHearingType : uint8
{
	NONE		UMETA(DisplayName = "None"),
	WALK		UMETA(DisplayName = "Walk"),
	RUN			UMETA(DisplayName = "Run"),
	CROUCH		UMETA(DisplayName = "Crouch"),
	REPEATER	UMETA(DisplayName = "Repeater"),
};

UENUM(BlueprintType)
enum class E_AISightCone : uint8
{
	NONE		UMETA(DisplayName = "None"),
	NOTSEEN		UMETA(DisplayName = "Not Seen"),
	BACKWARD	UMETA(DisplayName = "Backward"),
	PERIPHERAL	UMETA(DisplayName = "Peripheral"),
	WIDE		UMETA(DisplayName = "Wide"),
	NARROW		UMETA(DisplayName = "Narrow")
};

UENUM(BlueprintType)
enum class E_MantisAttackType : uint8
{
	NONE			UMETA(DisplayName = "None"),
	CLOSE_ATTACK	UMETA(DisplayName = "Close Attack"),
	SHIFT			UMETA(DisplayName = "Shift")
};

