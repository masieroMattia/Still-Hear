## Character
The logic of the structure between characters, player controllers and companion is as follows:
We have a base class ***StillHearCharacterBase***, which is the parent class from which every character must inherit.
Consequently, we have 2 character classes to represent our main character and its companion, respectively ***StillHearMainCharacter*** and ***StilllHearCompanionCharacter***.
The first class, ***StillHearMainCharacter***, is being spawned by the game mode (for which I temporarily did a class but 100% will not be the definitive one. For now it's just *EdoGameMode*.
Together with the character, the corresponding player controller is spawned, which inherits from ***StillHearPlayerController***. The camera, which will be discussed better in another file, is spawned by the player controller (for now...) on *Begin Play*.
The class of the companion, ***StilllHearCompanionCharacter***, is set to be possessed by an AI controller every time that it's placed in a scene or spawned, and the corresponding AI controller class is ***StillHearCompanionAIController***.
Generally speaking, when receiving an input from the player, the input is processed by the player controller which filters it and knows if it has to call a function which will impact the main character, or if it has to call a function on the AI Controller instead, which will respectively redirect it to the companion.
The character's abilities are handled by the **Gameplay Ability System**, which will be discussed more in another documentation file.
For this particular project, the movement itself was not treated as an ability so the input is just processed normally through the *AddMovementInput* function of the *Character Movement Component*.

### Nice things to know

 - ***StillHearCharacterBase***: implements the **Gameplay Ability System**'s functionalities.
 - ***StillHearMainCharacter***: handles the Spawn and Despawn of the companion and contains the methods to activate his own abilities. The player controller just calls them.

## Player Controller
 ***StillHearPlayerController*** manages the inputs for both the main character and the companion and activates input maps (for now). It's also responsible for setting the maximum distance between the player and the companion (for now it just stops movement if that happens). It constanly has a reference to the companion that is set respectively to null when it's not there, and to the companion when it's there. It has a class pointer to the camera class because it's responsible for spawning it (for now). 
 It has some utility functions such as:
  - **IsCompanionToTheLeft()/IsCompanionToTheRight()**: telling if the companion is to the left or the right of the character. Returns a bool.
  - **GetCompanionDistance()**: getting the distance between the character and the companion (float).
  - **AreCharactersAlmostOutOfScreen(int tolerance)**: checking if any of the two characters are out of screen bounds (useful for example for the camera to know if it should get back to make space or come in). The tolerance are the amount of pixels to not consider in the calculation (for example, if the tolerance is 50, it means the camera will start to get farther from the "focus point" if any of the characters reached the screen bounds - 50 pixels).

## AIController (for companion)
***StillHearCompanionAIController*** manages the inputs for the companion, which are always received as a method call from the Player Controller. Other than this it just has some useful parameters to be set from editor, linked to the usage of the **GAS**. In particular, it has a **Gameplay Tag** that can be set to be sent to the main character after the companion has been recalled by the recall ability, and also has a reference to the character in order to know where to go once it's been recalled.

