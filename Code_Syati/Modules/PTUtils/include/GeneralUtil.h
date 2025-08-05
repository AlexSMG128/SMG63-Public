#pragma once

#include "syati.h"

namespace pt {
    template<typename R>
    R* accessMember(void* ptr, size_t offset);
    void* accessMember(void* ptr, size_t offset);
    template<typename R>
    const R* accessMember(const void* ptr, size_t offset);
    const void* accessMember(const void* ptr, size_t offset);
    void setSubBGMState(s32 state, u32 fadeTime);
    s32 getIOSVersion();
    s32 getIOSRevision();
    void str2wcsfullwidth(wchar_t* dest, s32 src);
};