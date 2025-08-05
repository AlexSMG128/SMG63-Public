#ifndef __REVOLUTION_VI_H
#define __REVOLUTION_VI_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VI_NTSC 0x00
#define VI_PAL 0x01
#define VI_MPAL 0x02
#define VI_DEBUG 0x03
#define VI_DEBUG_PAL 0x04
#define VI_EURGB60 0x05

u32 VIGetTvFormat();

#ifdef __cplusplus
}
#endif

#endif // __REVOLUTION_DVD_H
