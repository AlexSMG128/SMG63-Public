# SMG2 OpenGhost
### Cosmic Races in SMG2


This is a reimplementation of Cosmic Races from Super Mario Galaxy. This is not a 1-to-1 port.

Below is a list of everything included:
- The ObjectData archives for Cosmic Mario & Cosmic Luigi
  - Also includes custom Rolling Rocks for use with the Rock Mushroom
- The "GhostPlayer" object in the Editor
- The "Ghost" Comet type in the Scenario
- A decorative "GhostPowerUpRock" object, which you will have to add to the ProductMapObjDataTable before using. (Class type is up for you to decide)

Below is a list of other things that you will need:
- [GalaxyGST](https://github.com/SunakazeKun/galaxygst)
- [Dolphin Emulator](https://dolphin-emu.org/)
- A level editor
- A camera editor


## Instructions
To create a custom cosmic race, follow the GalaxyGST steps to create a Luigi Ghost. (Play as Mario to record a Cosmic Mario, and Luigi to record a Cosmic Luigi)<br/>
Once that is done, instead of deleting the GST recorder object, rename it to GhostPlayer.<br/>
Set the ObjArg0 to the GST number you want to use (1 for example) and then rename your GST file from GalaxyGST to "GhostPlayerData01" (or "GhostPlayerData01Luigi" for Luigi)<br/>
Put the GST file into your Galaxy's Ghost.arc ("IslandFleetGalaxyGhost.arc" for example) inside the gst folder (which goes on the root. If it doesn't exist, you may create it).

Once all these steps are complete, the Ghost will work in-game.

## Guidelines for Recording Ghosts
These guidelines are only recommendations based on what the ghost object supports doing.
- Do not take damage. (The damage animation playback isn't proper)
- Do not interact with Enemies. (There is no interaction between enemies and Cosmic Mario aside from *maybe* some starbits appearing)
- Do not use Starbit Crystals (the blue ones on launch star paths). (The Cosmic player can activate these even if the normal player is not in a launch star.)
- Do not ride Yoshi. (There is no Yoshi support at this point in time.)
- Do not carry objects. (there's no guarentee that it will work properly)