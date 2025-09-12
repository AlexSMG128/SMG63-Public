# SMG2 SimpleNPC

SimpleNPC allows the addition of new NPCs to Super Mario Galaxy 2, with no extra programming required!

## NPC Setup
Everything for a SimpleNPC actor is handled via the object archive for the NPC (the only exception being [NpcGoods](#NpcGoods)). To allow this, some BCSVs have been edited, or added.

### InitActor
Some new InitFunction values were added.

| InitFunction Value | Required Inputs |
| ------------- | ------------- |
| NpcActionWait | Data = Sets the animation name to use for Waiting |
| NpcActionWaitTurn | Data = Sets the animation name to use for Turning while Waiting |
| NpcActionTalk | Data = Sets the animation name to use for Talking  |
| NpcActionTalkTurn | Data = Sets the animation name to use for Turning while Talking |
| NpcActionSpin | Data = Sets the aniation name to use when getting spun by the player's spin attack |
| NpcActionTrample | Data = Sets the animation name to use when getting jumped on by the player |
| NpcActionPointing | Data = Sets the animation name to use when... (?) |
| NpcActionReaction | Data = Sets the animation name to use when... (?) |
| NpcRotate | Data = `o` to enable NPC Rotation (default) or `x` to disable |
| NpcRotateTalk | Data = `o` to enable NPC Rotation during talk (default) or `x` to disable |
| NpcYoshiTarget | Data = `o` to enable Yoshi target support |
| NpcTalkDist | Param00F32 = The range around the NPC in which the player can talk to it |
| NpcGoods | If a row with this exists in the BCSV, [NpcGoods](#NpcGoods) are initialized |
| NpcAnimScale | Data = name of the joint to apply the [AnimScaleParam](#AnimScaleParam-NEW) to. |

### AnimScaleParam (NEW)
Creates an `AnimScaleParam` instance for the bone specified in [InitActor](#InitActor)
| Field | Description |
| ------------- | ------------- |
| Param00F32 | *Sets AnimScaleParam->_0* |
| Param01F32 | *Sets AnimScaleParam->_4* |
| Param02F32 | *Sets AnimScaleParam->_8* |
| Param03F32 | *Sets AnimScaleParam->_C* |
| Param04F32 | *Sets AnimScaleParam->_10* |
| Param05F32 | *Sets AnimScaleParam->_14* |
| Param06F32 | *Sets AnimScaleParam->_18* |
| Param07F32 | *Sets AnimScaleParam->_1C* |
| Param08F32 | *Sets AnimScaleParam->_20* |
| Param09F32 | *Sets AnimScaleParam->_24* |
| Param10F32 | *Sets AnimScaleParam->_28* |
| Param11Int | *Sets AnimScaleParam->_2C* |
| Param12F32 | *Sets AnimScaleParam->_30* |

### NpcBehavior (NEW)
Allows the creation of various Behaviors for your NPC to use (similar to how Toads have different Behaviors)

| Field | Description |
| ------------- | ------------- |
| Type | The base Behavior to use. Refer to the [Behavior Types](#Behavior-Types) list below. |
| Param00Str | The first parameter specific to the type |
| Param01Str | The second parameter specific to the type |
| Param02Str | The third parameter specific to the type |

#### Behavior Types
| Name | Description | Inputs |
| ------------- | ------------- | ------------- |
| NoTurn | The NPC will not rotate to face Mario, and will have a fixed animation. | Param00Str = Fixed animation name<br/>Param01Str = *Unused*<br/>Param02Str = *Unused* |
| Single | The NPC will have a fixed animation. | Param00Str = Fixed animation name<br/>Param01Str = *Unused*<br/>Param02Str = *Unused* |
| WaitTurnWalk | Changes which animations will be used for Waiting, Turning, and Walking | Param00Str = Name of the Wait animation (*Optional. leave Blank to use the value from [InitActor](#InitActor)*)<br/>Param01Str = Name of the Turn animation (*Optional. leave Blank to use the value from [InitActor](#InitActor)*)<br/>Param02Str = Name of the Walk animation (*Optional. leave Blank to use the value from [InitActor](#InitActor)*) |

### NpcGoods
You can define accessories (called "goods") for your NPC using the vanilla game's NpcGoods system. (Located in `ObjectData/NPCData.arc`. Name your BCSV the same name as your NPC name.)

Refer to the [Luma's Workshop Wiki](https://www.lumasworkshop.com/wiki/NPCData) for more information on how to use the vanilla NpcGoods system.

### Object Arguments
In the editor, this object has the following Object Arguments

| Argument | Description |
| ------------- | ------------- |
| ObjArg0 | Chooses a frame in ColorChange.brk (BTK not supported) |
| ObjArg1 | Chooses a frame in TexChange.btp |
| ObjArg2 | Chooses a Behavior from the [Behavior List](#NpcBehavior-NEW) |
| ObjArg3 | Chooses an item from the [NpcGoods](#NpcGoods) |
| ObjArg4 | Instance Specific properties (bitfield)<br/>0 = None<br/>1 = Create a "TakeOutStar" (Requires [GLE](#Galaxy-Level-Engine-support) to be able to spawn stars at the moment) |

### Animations
Animations support the use of `ActorAnimCtrl` to define Actions (which can use multiple animation types at once. Example: Wait.bck and Blink.btp). Using this BCSV is optional, and if it isn't present, the game will just use the animation names directly (Example: Wait.bck).

Binary Animation Tables (.banmt) are also supported. These are BCSV files (despite the unique extension) which allow interpolation when switching animations

### Sounds
Sounds from the MSBT are supported (because of course they are)

In addition, you can use a Binary Action Sound (.bas) file to play sfx based on animations.

You can also add entries to `ActionSound` so long as you set the "TriggerType" to `0`, and put the name of the animation it's tied to in the "TriggerName" field.<br/>SimpleNPC has no specific UniqueName `ActionSound` entries right now, so you can name it whatever you want.

### Particle Effects
Effects can be added to the AutoEffectList. Added effects can only be tied to animations (using the `AnimName` field), as SimpleNPC does not have any explicit particle emittors.

### Shadows
Shadows can be added with the vanilla Shadow.bcsv inside the NPCs `InitActor` folder.

### Collisions
NPCs can use `InitCollision` to use a KCL file, and also use `InitSensor` to use HitSensors (use `Npc` as the HitSensor Type).


## Registering a SimpleNPC
Simply edit your ProductMapObjDataTable to add an entry for your NPC using `SimpleNPC` as the class.

## [Galaxy Level Engine](https://github.com/SuperHackio/GalaxyLevelEngine) support
If this module is compiled with the Galaxy Level Engine API module, all SimpleNPC instances will have access to GLE's [Global Message Flow](https://github.com/SuperHackio/GalaxyLevelEngine/wiki/NPC#msbf) system. Currently it is required if you want to be able to spawn power stars.
