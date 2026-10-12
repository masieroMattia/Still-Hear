#pragma once

UENUM(BlueprintType)
enum class EAttachPoint : uint8
{
	ROOT		UMETA(DisplayName = "Root"),
	MESH		UMETA(DisplayName = "Mesh"),
	COMPONENT   UMETA(DisplayName = "Component")
};
