# Animation
Anything we did in C++ with animation.
## AnimNotifies
### SendGameplayEvent
It's meant to be put on a montage, and just sends a Gameplay Event with a specific tag (which can be put in editor) to the owner of the mesh of the montage that is playing, when the montage reaches the notify. Incredibly useful and used to time abilities correctly.
