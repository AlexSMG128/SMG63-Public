#pragma once

#include "syati.h"
#include "Game/Screen/CountUpPaneRumbler.h"
#include "Game/Screen/SysInfoWindow.h"
#include "Game/Screen/PauseMenu.h"
#include "BlueCoinList.h"

const wchar_t counterPictureFonts[] = {
0x000E, 0x0003, 0x0055, 0x0002, 0x005A, // Blue Coin Icon
0x000E, 0x0003, 0x0010, 0x0002, 0x0010, // X Icon
0x000E, 0x0006, 0x0001, 0x0008, 0x0000, // Counter
0x0000, 0x0000, 0x0000
};

const wchar_t counterPictureFonts1[] = {
0x000E, 0x0003, 0x0055, 0x0002, 0x005A, // Blue Coin Icon
0x000E, 0x0003, 0x0010, 0x0002, 0x0010, // X Icon
0x000E, 0x0006, 0x0001, 0x0008, 0x0001, // Counter
0x0000, 0x0000, 0x0000
};

const wchar_t counterPictureFonts2[] = {
0x000E, 0x0003, 0x0055, 0x0002, 0x005A, // Blue Coin Icon
0x000E, 0x0003, 0x0010, 0x0002, 0x0010 // X Icon
};


void PauseMenuIDListControls(PauseMenu* pPauseMenu);

extern "C" {
    // unkPauseMenuReturnToSelect__FP9PauseMenu
    void __kAutoMap_80487540(PauseMenu*); // sub_80487540
}