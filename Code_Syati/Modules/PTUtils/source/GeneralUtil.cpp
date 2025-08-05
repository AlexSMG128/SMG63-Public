#include "GeneralUtil.h"

/*
* Authors: Lord-Giganticus, Bavario
*/

extern "C" {
    void *getSubBgm__7AudWrapFv();
}

namespace pt {
    template<typename R>
    R* accessMember(void* ptr, size_t offset) {
        u8* p = static_cast<u8*>(ptr);
        return static_cast<R*>(&p[offset]);
    } 
    void* accessMember(void* ptr, size_t offset) {
        u8* p = static_cast<u8*>(ptr);
        return static_cast<void*>(&p[offset]);
    }
    template <typename R>
    const R* accessMember(const void* ptr, size_t offset) {
        const u8* p = static_cast<const u8*>(ptr);
        return static_cast<const R*>(&p[offset]);
    }
    const void* accessMember(const void* ptr, size_t offset) {
        const u8* p = static_cast<const u8*>(ptr);
        return static_cast<const void*>(&p[offset]);
    }

    asm void setSubBGMState (s32 state, u32 fadeTime) {
        stwu      r1, -0x10(r1)
        mflr      r0
        stw       r0, 0x14(r1)
        stw       r31, 0xC(r1)
        mr        r31, r4
        stw       r30, 8(r1)
        mr        r30, r3
        bl        getSubBgm__7AudWrapFv
        cmpwi     r3, 0
        beq       End
        lwz       r12, 0(r3)
        mr        r4, r30
        mr        r5, r31
        lwz       r12, 0x30(r12)
        mtctr     r12
        bctrl
    End:
        lwz       r0, 0x14(r1)
        lwz       r31, 0xC(r1)
        lwz       r30, 8(r1)
        mtlr      r0
        addi      r1, r1, 0x10
        blr
    }

    // Inspired by libogc
    s32 getIOSVersion () {
	    u32 vercode = *((u32*)0x80003140);
	    u16 version = vercode >> 16;
	    return version;
    }
    s32 getIOSRevision () {
        u32 vercode = *((u32*)0x80003140);
	    u16 revision = vercode & 0xFFFF;
	    return revision;
    }

    void str2wcsfullwidth(wchar_t* dest, s32 src) {
        wchar_t buf[32];
        swprintf(buf, 32, L"%d", src);

        size_t len = wcslen(buf);

        for (s32 i = 0; i < len; i++) {
            dest[i] = buf[i] + 0xFEE0;
        };
        dest[len] = 0;
    }
};