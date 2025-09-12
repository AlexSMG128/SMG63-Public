# SMG2 BossKameckReturns
### Kamella in SMG2

This is a remaster/port of Kamella from Super Mario Galaxy. It includes new SMG2 specific features while still being able to recreate the original fights.

Below is a list of everything included:
- The ObjectData archives for Kamella.
- The "BossKameck" object in the editor
- The "BossKameckMoveRail" object in the editor
- The "BossKameckFollowerGroup" object in the editor
- The "KameckJetTurtle" object in the editor
- The "BossKameckBarrier" class for the ProductMapObjDataTable
- A set of BCSV files for creating custom battle data
- 2 sets of pre-made battle data, copying the fights in Super Mario Galaxy
- An edited Effect.arc file that includes particle effects for all included objects

Below is a list of other things that you will need:
- A level editor

## Credits
- @shibbo - Decompiling all of BossKameck
- @sunakazekun - Original fixes for Koopa Shell magikoopas
- @superhackio - Ported and enhanced SMG1 code to SMG2

## New Features
- Custom beam attack patterns
- Custom minion spawns
- Custom battle lengths
- BCSV based battle setup
- Custom barriers
- Koopa Shell casting magikoopas
- The power star inside her wand has support for different colours. This should work in both Vanilla and [Galaxy Level Engine](https://github.com/SuperHackio/GalaxyLevelEngine )

## Setup
The setup for Kamella is relatively simple.
- Place `BossKameck` into the level. The position matters, as the battle cutscenes will use this position as their base position.
- Set `obj_arg0` to a number, which will select what Battle Data archive to load from ObjectData. (BossKameckVsX, where X is ObjArg0)
- Set `obj_arg1` to a Power Star ID. Optional. Kamella will activate her SW_DEAD when killed, so that may be used instead.

Of course, with all the new features, you will need some Battle Data. For this example, we will assume we are using `BossKameckVs1`. Refer to the sections below for the Battle Data setup.

Once the Battle Data is setup, you will need to setup the following:
- `BossKameckMoveRail`:
    - You will need one of these for each boss phase.
    - Link them to Kamella using their Generator Id.
    - Use the Path ID to determine what path Kamella will have.
    - Set `obj_arg0` to the Phase Index, starting from 0.
- `BossKameckFollowerGroup`
    - You can have as many or as little as you want.
    - Set `obj_arg0` to a unique ID, which you can then reference in `BattleData.bcsv`
    - Link them to Kamella using their Generator Id.
    - Link any amount of objects that are of the `Kameck` class to this using their Generator Id.
- Any named Barrier objects
    - You can have only one Barrier object per phase.
    - Barriers are identified by name, so you can only have one barrier with a given name per Kamella.
    - Barriers can be literally any object that is a `LiveActor`, such as a `SimpleMapObj`, or the included `BossKameckBarrier`

#### BattleData.bcsv
This BCSV dictates how many phases Kamella has, and how she behaves for each.<br/>One phase lasts until she takes damage

| BCSV Field | Description |
| :---: | ------------- |
| FollowerGroupId<br/>(`0xB2AACDDC`) | Selects a `BossKameckFollowerGroup` to use for this phase.<br/>Set to -1 to disable. |
| AllowFollowerReappear<br/>(`0xBAB6350F`) | 0 = The selected `BossKameckFollowerGroup` will not be respawned. (A `PowerUpDemo` is required to spawn the followers initially)<br/>1 = The selected `BossKameckFollowerGroup` will respawn after all Kamecks spawned are dead, and 3 attacks from Kamella have occured. |
| AnimName<br/>(`0xD46BA45C`) | An animation name to play when starting this phase.<br/>Leave blank to disable. |
| Barrier<br/>(`0x4F5D3B97`) | The name of a linked Barrier object. `appear` is called when the phase begins, and `kill` will be called when the next phase either disables all barriers, or switches to another barrier.<br/>Leave blank to disable all barriers. Keep the name of the previous barrier to have nothing change. |
| MovementType<br/>(`0xD46768A9`) | 0 = Kamella will simply float along her current path.<br/>1 = Kamella will user her teleport move, which is much faster and more sporadic. |

#### BeamPatternLvX.bcsv
You will need one BCSV for each kamella phase, starting with `BeamPatternLv1`, then going to `BeamPatternLv2`, and so on.<br/>Kamella will start each phase at the first BCSV entry, and work her way down in order, restarting once complete.

| BCSV Field | Description |
| :---: | ------------- |
| Beam<br/>(`0x001F87CF`) | The numerical ID of the Kameck Beam Type. Vanilla values are as follows:<br/>1 = Fireballs<br/>2 = Koopa Shells<br/>3 = Mini Goombas<br/>4 = Li'l Cinders |
| Count<br/>(`0x03E460EF`) | The number of items to spawn.<br/>**MAX LIMIT 3** |

<sub>if there was a module that added more Kameck Beam Types, you could access them via their IDs<sub/>














 
