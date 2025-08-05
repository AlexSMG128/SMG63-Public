# SMG2 FPS Display
This SMG2 mod will add a FPS Display to your game. Very useful for mod developers to make sure that the levels run at a smooth 60fps.


https://github.com/SuperHackio/SMG2-FPSCounterLayout/assets/44330283/b22fa8ff-d5bd-4663-b206-e7e72a36df18

***Note: Dolphin doesn't emulate game lag, so this layout is useless on emulator.***

## Supported Regions

The following regions and video modes are supported:
| Region | TV Type / TV Resolution |
| :---: | -------- |
| USA | `Standard TV (480i)` <br/>`EDTV or HDTV (480p)` |
| PAL | `50Hz (576i)`<br/>`60Hz (480i)`<br/>`EDTV or HDTV (480p)` |
| JAP | `プログレッシブ`<br/>`インターレース` |
| KOR | `프로그레시브`<br/>`인터레이스` |
| TAW | `プログレッシブ`<br/>`インターレース` |


## How it works
Every frame will have it's FPS calculated and displayed to your screen in the layout on the top of the screen.<br/>The background colour of the layout will change as the number of frames changes.<br/>Below is a table of what the colours mean:
| Colour Range | Frame Range<br/>(60 FPS) | Frame Range<br/>(50 FPS) | Description |
| :---: | :-----: | :---: | ---- |
| 🔵 -> 🟢 | 60 - 55 | 50 - 45 | This is the perfect range. If you mod ticks inbetween here, you should be good to go. |
| 🟢 -> 🟡 | 55 - 50 | 45 - 40 | This is a good range. Even vanilla SMG2 drops this low at times. Your mod can drop to this range at times, but preferably as rare as possible. |
| 🟡 -> 🔴 | 50 - 40 | 40 - 35 | This range is rather bad. While SMG2 does appear to drop to 40 FPS at exceptional occurences, you really don't want this to happen in your mod. |
| 🔴 -> ⚫ | 40 - 00 | 35 - 00 | Reaching this range is an achievement in its own regard. |

### New to Version 2
- You can press (1) on the Wii Remote to show/hide the layout
- You can press (2) on the Wii Remote to change FPS Calculation modes (listed below)

FPS Calculation modes:
| Name | Icon | Description |
| --- | :-----: | ---- |
| Standard | ![60fps mode](https://raw.githubusercontent.com/SuperHackio/SMG2-FPSCounterLayout/main/Mode0.png)<br/>![50fps mode](https://raw.githubusercontent.com/SuperHackio/SMG2-FPSCounterLayout/main/Mode3.png) | To avoid flickering, there is a deadzone between 60.00 and 59.00 (50.00 and 49.00 in 50Hz). While 59.00 can still show up, anything above 59.00 will be rounded to 60.00. |
| Precise | ![Precise mode](https://raw.githubusercontent.com/SuperHackio/SMG2-FPSCounterLayout/main/Mode1.png) | Full precision is used on the display, meaning all possible values will be shown. |
| Rounded | ![Rounded mode](https://raw.githubusercontent.com/SuperHackio/SMG2-FPSCounterLayout/main/Mode2.png) | Shows whole numbers only. |

## Supported Extensions
- [Galaxy Level Engine](https://github.com/SuperHackio/GalaxyLevelEngine) :heavy_check_mark:

## Build Instructions
This mod is a **[Syati Module](https://github.com/SMGCommunity/SyatiModuleBuildTool)**.<br/>Needs the [Syati_GlobalLayouts](https://github.com/SMGCommunity/Syati_GlobalLayouts) module in order for it to load in-game.

The required game files can be found in the "disc" folder.
