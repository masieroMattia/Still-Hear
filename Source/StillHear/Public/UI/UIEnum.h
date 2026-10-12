#pragma once

// Defines how the widget handles input
UENUM(BlueprintType)
enum class E_WidgetInputMode : uint8
{
    Default,
    GameAndMenu,
    Game,
    Menu
};

// Defines what content to display on the button
UENUM(BlueprintType)
enum class EButtonContentMode : uint8
{
    TextOnly     UMETA(DisplayName = "Text Only"),
    IconOnly     UMETA(DisplayName = "Icon Only"),
    TextAndIcon  UMETA(DisplayName = "Text and Icon")
};