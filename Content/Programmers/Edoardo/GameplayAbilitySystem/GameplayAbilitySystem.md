# Gameplay Ability System
For this project we decided to take advantage of the power and organization of the Gameplay Ability System. The documentation is going to be divided in the same way the Ability System is divided: Abilities, Effects, Cues, Attributes. If the category is missing it means it's either not present in the project (for example if the character won't have any  useful attribute to be modified, maybe we won't have attributes at all), or it's not been implemented yet. In the end a list of tags used will be displayed, with their role described briefly.
Before we dive in, a little description of how GAS has been implemented in the project. First of all, we have the ***StillHearAbilitySystemComponent*** which inherits directly from **UAbilitySystemComponent**. For now it does... Nothing useful, but better to have it than not have it (it actually does something but it's some stuff needed for multiplayer... It's not really needed for single player so i'll ignore putting it here).
All the characters in the game derive from a base class, which is ***StillHearCharacterBase***, in which the Ability System Component is set up and added to the character. It also allows the usage of a couple of handy functions that grant and remove abilities from the character. They accept arrays as input, so to be able to assign and remove multiple abilities at once.
## Abilities
### SetCompanionFree
Plays a montage which sends an event that triggers the character to spawn the companion. After this, a Gameplay Tag will be granted to the character thanks to a Gameplay Effect (which only grants the tag to the character and does nothing else). It works together with the recall ability and can't be executed if the character has the tag that this ability puts on him.
### CompanionRecall
Plays a montage which sends an event that triggers the player controller to tell the companion to make its way to the character. Once the companion reaches the character, it sends an event that triggers the character to make him despawn. After this, it removes from the character the tag that was set from the SetCompanionFree ability, so he can use it again. This ability can't be used if the character doesn't have the tag which the SetCompanionFree ability gives him. 
## Effects
### GrantTagsToActor
It's just a parent class that sets the duration of the effect to infinite. Since it's so simple, all the effects that only have to grant a tag to an actor and nothing else are better just created in blueprint, as children of this class. It's just too much simpler.
## Cues
## Attributes




