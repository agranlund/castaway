/*
 * Castaway
 *  (C) 1994 - 2002 Joachim Hoenig, Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */
#ifndef CONFIGH
#define CONFIGH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ATARI
#include <mint/mintbind.h>
#include <mint/osbind.h>
#endif

#define BIG_ENDIAN
#define INLINE          static inline
#define _stricmp        stricmp

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;


#define VIDEO_OFF			0
#define VIDEO_ST			1
#define VIDEO_TT			2
#define VIDEO_FALCON		3
#define VIDEO_VAMPIRE		4
#define VIDEO_NOVA			5

#define SOUND_OFF			0
#define SOUND_PSG			1
#define SOUND_DMA			2

// castaway.c
#define MEM_CHIP 0
#define MEM_FAST 1
extern uint32 AllocateMem(uint32 size, uint32 align, uint16 type);
extern int InsertDisk(int num);
extern int InitEmulator(int args, char* argv[]);
extern void QuitEmulator();

// host.c
extern uint32 HostAlloc(uint32 size, short type);
extern void HostFree(uint32 ptr, short type);
extern void HostEvents();
extern void HostVsync();
extern void HostSound();
extern short HostInit();
extern void HostExit();

#ifndef DISTBUILD
#define DBG(...)    { printf( __VA_ARGS__); printf("\n\r"); }
#define HALT(...)   { DBG(__VA_ARGS__); exit(1); }
#else
#define DBG(...)
#define HALT(...)
#endif

#ifndef DISTBUILD
    #ifndef DEBUG_SPEED
    #define DEBUG_SPEED         0
    #endif
    #ifndef DEBUG_CPU
    #define DEBUG_CPU           0
    #endif
#else
    #define DEBUG_SPEED         0
    #define DEBUG_CPU           0
#endif

#define DISABLE_GRAPHICS        (DEBUG_SPEED | DEBUG_CPU)
#define FRAMEPACING_DEFAULT     (1 & !DEBUG_SPEED)

#ifndef COLDFIRE
    #define CHAINED_OPS         0
#else
    #define CHAINED_OPS         0
#endif


/*
 * Atari ST emulator defaults
 */
#define MHZ                     8
#define CYCLES_PER_RUN          ((400 * MHZ) / 8)
#define CYCLES_PER_LINE         ((512 * MHZ) / 8)
#define CYCLES_BORDER           ((96 * MHZ) / 8)

#define START_VISIBLE_SCREEN    64
#define END_VISIBLE_SCREEN      264
#define START_VBLANK            313

#define MEMBASE                 0x00000000L
#define MEMSIZE                 (1024*1024)
//#define MEMSIZE                 (512*1024)
#define IOBASE                  0x00ff8000L
#define IOSIZE                  0x00008000L /* 32k */
#define SVADDR                  0x00000800L

#define MONITOR                 0               /* 0=color 320x200 or 2=monochrome 640x400 */
#define DISKA                   "blank.st"      /* Disk A RomImage */
#define DISKB                   "blank.st2"     /* Disk B RomImage */
#define CARTRIDGE               "cart.img"      /* cartridge image RomImage */
#define ROM                     "tos.img"       /* ROM image RomImage */
#define SIDES                   2               /* disk sides */
#define TRACKS                  80              /* tracks on disk */
#define SECTORS                 9               /* sectors per track */
#define SECSIZE                 512             /* byte per sector */
#define TIMER                   0               /* 0=normal (200Hz), 2=slow (100Hz) */
#define NO_BLITTER                              /* Do not emulate Blitter */
#define NO_RTC                                  /* Do not emulate Real-Time-Clock */

#undef CHKADDRESSERR        /* if set, unaligned access will raise address exception */
#undef CHKTRACE             /* if set, the trace bit works (slower). */
#undef DEBUG                /* Debug */
#undef DISASS               /* Active Disassembling  option */
#undef DBGTRACE             /* Active Debug Statistique */
#undef DETECT_PREFETCH      /* Active Prefetch detection traced in file */

extern int opt_framepacing;


/*
 * Debug options
 */
#ifdef DEBUG
    #ifndef CHKADDRESSERR
    #define CHKADDRESSERR
    #endif
    #define VERBOSE 0x1
    #define TRACEBACK 20000
    #undef INTERNALTRACE
    #define DBG_OUT if (verb_on) printf
    #define DBG_STOP if (stop_on) Stop
    #define NO_TIMER
    #define ON_TRAP(number) if (number == 33) {stop_on++;};
    #define ON_UNMAPPED(address, value)
    #define ON_NOIO(offset, value)
    #define ON_WRITE(address, value)
    extern int      trace_on;
    extern int      stop_on;
    extern int      verb_on;
    extern void     SaveState(unsigned short inst);
    extern void     Stop(void);
#else
    #define ON_TRAP(number) 
    #define ON_UNMAPPED(address, value)
    #define ON_NOIO(address, value)
    #define ON_WRITE(address, value)
#endif
#endif

