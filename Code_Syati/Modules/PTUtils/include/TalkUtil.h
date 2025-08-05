#pragma once

#include "syati.h"
#include "Game/NPC/TalkMessageCtrl.h"

namespace pt {
    void setMessageArg(TalkMessageCtrl *pTalkCtrl, const wchar_t *pText);
    void setTextBoxArgStringNumberFontRecursive(LayoutActor* pLayout, const char* paneName, u16 num, s32 index);
};
