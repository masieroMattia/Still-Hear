# StillHear

> A third-person stealth game built in **Unreal Engine 5.5.4**, where sound is both your

> tool and your enemy.

---

## 📌 Overview

StillHear is an audio-driven stealth game. The player, aided by a floating companion, must

navigate levels while managing how much noise they make — enemies such as the **Mantis**

(sight + hearing) and the **Worm** (ground vibration) hunt by what they perceive.

For a full breakdown of the C++ systems, see the **[Technical Documentation](./Documentation/StillHearDocumentation.md)**.

Some parts of the code have a more in-depth documentation which you can check in the [Documentation](./Documentation/) folder.

<!-- ^ update this link to wherever you place the code documentation -->

## 🗺️ Opening the Project

Open the project in the Unreal Editor. The main game map is:

```

StillHear_Level

```

## ▶️ Testing a Specific Section

The game is built using a level streaming flow managed by a **Scene Manager**. To jump in and
test a specific area, follow these steps:

1. **Right-click** in the viewport at the point where you want the player to spawn.

2. In the context menu, use the dedicated tool (near the **bottom** of the list) that **moves the Player Start to the clicked location**.

3. Search for **`SceneManager`** in the World Outliner and select it.

4. Open the Scene Manager's **Level Streaming** section and make sure the level you want to spawn in is present in the list of **loaded levels**.

5. Toggle the Scene Manager's **"start in menu mode"** option on or off, depending on whether you want the game to boot into the main menu or straight into gameplay.

6. Press **Play**. ✅
