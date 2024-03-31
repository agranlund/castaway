#include "config.h"
#include "68000.h"
#include "mem.h"
#include "st.h"
#include <mint/mintbind.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>
#include <mint/sysbind.h>
#include <mint/falcon.h>
#include "gem.h"

// alignment. compare mapfile.
//
// this time align.2 was faster...
// probably depends on the initial address of text,data,bss which I think is word aligned.
//
// make own loader, or make emutos align the sections to 4bytes?
//

//
// check all global bss/data and make sure longs are 4byte aligned!
// re-arrange and pad as needed.
//
// also put related stuff close for better cache-line usage
// the huge jumptable should be first or last and not in the middle of things.
//


//             Coremark    Cps  dhry2120
// 68030 @ 50        23      3
// CT60 @ 60                    110000
// CT60 @ ??                13
// CT60 @ 85        193
// Vampire @ 85     214     16  147333 (Amiga sysinfo)
// CT60 @ 95        217
// CT60 @ 105       232
// Firebee          467     50  466000
//

//14.69
//14.95 no host events

// dhrystone:

// CT60 @ 66        


#define DEBUG_ALIVE                 0
#define DEBUG_NOPAL                 0

#if DEBUG_ALIVE
    #define VAMPIRE_AMIGA_SCREEN    0
#else
    #define VAMPIRE_AMIGA_SCREEN    1
#endif


extern short turbo;

// bss
uint32 oldPaletteExt[256];
uint16 oldPalette[16];
uint8 oldSoundRegs[16];
uint16* oldPhysbase;
uint16 oldRez;
void(*oldKeyboardVector)(void);
void(*oldMouseVector)(void*);
void(*oldJoystickVector)(void*);
uint32* oldVblQ;
uint32* newVblQ;
uint16  oldVblS;
uint16  newVblS;
uint8 oldConterm;
uint8 initstate;
short vdo;
short snd;
short mch;
uint8 keys[128*2];
uint8 joykeys[128];
uint8 joys[2*2];
uint16* screenPtr[2];
int screenIdx;
int rasterIdx;
int swapRasters;
short hostcpu;
short vampire;
uint16 modevals[4];
uint8* RasterBuffer[2];

#define EnableInterrupts()   { __asm__ volatile("move.w #0x2300,sr\n\t" : : : "cc"); }
#define DisableInterrupts()  { __asm__ volatile("move.w #0x2700,sr\n\t" : : : "cc"); }

uint32 HostAlloc(uint32 size, short type)
{
    uint32 ptr = 0;
    if (type & MEM_FAST)
        ptr = Mxalloc(size, 3);
    if (ptr == 0)
        ptr = Mxalloc(size, 0);
    return ptr;
}

void HostFree(uint32 ptr, short type) {
    Mfree(ptr);
}

static inline void WaitVBL() {
    Vsync();
}


extern void VecRaster16();
extern void VecRasterEnd();
extern uint16* RasterList;

void VecVbl(void)
{
    if (vid_rasters)
    {
        *((volatile uint8*)0xFFFFFA07) &= ~1;       // disable timerb
        *((volatile uint8*)0xFFFFFA0B) &= ~1;       // clear pending

        if (swapRasters)
        {
            rasterIdx = (rasterIdx + 1) & 1;
            swapRasters = 0;
        }
        RasterList = (uint16*)RasterBuffer[rasterIdx];
        uint32 func = *((uint32*)RasterList); RasterList += 2;
        uint16 count = *RasterList++;

        if (func != 0 && count == 0)
        {
            for (short i=0; i<16; i++)
                Setcolor(i, *RasterList++);

            func = *((uint32*)RasterList); RasterList += 2;
            count = *RasterList++;
            if (func != 0 && count != 0) {
                *((volatile uint32*)0x120) = func;      // func
                *((volatile uint8*)0xFFFFFA1B) = 0;     // stop
                *((volatile uint8*)0xFFFFFA21) = count; // counter
                *((volatile uint8*)0xFFFFFA1B) = 8;     // event count
                *((volatile uint8*)0xFFFFFA13) |= 1;    // mask enable
                *((volatile uint8*)0xFFFFFA07) |= 1;    // enable timerb
            }
        }
    }
}


void HostVblank()
{
#if DISABLE_GRAPHICS
    return;
#endif

    // don't flip more than once per frame if going fast
    if (opt_framepacing == 0) {
        static long lastframe = 0;
        long curframe = *((volatile long*)0x466);
        if (curframe == lastframe)
            return;
        lastframe = curframe;
    }
#if DEBUG_ALIVE
    static short curframe = 0;
    curframe++;
    Setcolor(0, curframe & 1 ? 0x0F00 : 0x000F);
#endif    


    #if VAMPIRE_AMIGA_SCREEN
    if (vampire) {
        Amiga_Blit();
        return;
    }
    #endif

    // resolution change
    if (vid_flag)
    {
        static uint8 oldrez = 0;
        if (vid_shiftmode != oldrez) {
            oldrez = vid_shiftmode;
            uint16 mode = modevals[vid_shiftmode];
            switch (vdo) {
                case 0:
                case 1:
                    Setscreen(-1, -1, mode);
                    break;
                case 2:
                    EsetShift(mode);
                    break;
                case 3:
                    VsetMode(mode);
                    break;
            }
        }
#if !DEBUG_NOPAL
        if (vid_flag & VIDFLAG_PALETTE)
        {
            if (vid_rasters)
            {
            
                DisableInterrupts();

                uint16* p = (uint16*)RasterBuffer[(rasterIdx + 1) & 1];
                static uint16 colors[16];
                short lastline = 0;
                for (short i=0; i<200; i++)
                {
                    uint16 flag = vid_colf[i];
                    if (flag || (i == 0)) {
                        if (flag & (1 <<  0)) { colors[ 0] = vid_colr[(i << 4) +  0]; }
                        if (flag & (1 <<  1)) { colors[ 1] = vid_colr[(i << 4) +  1]; }
                        if (flag & (1 <<  2)) { colors[ 2] = vid_colr[(i << 4) +  2]; }
                        if (flag & (1 <<  3)) { colors[ 3] = vid_colr[(i << 4) +  3]; }
                        if (flag & (1 <<  4)) { colors[ 4] = vid_colr[(i << 4) +  4]; }
                        if (flag & (1 <<  5)) { colors[ 5] = vid_colr[(i << 4) +  5]; }
                        if (flag & (1 <<  6)) { colors[ 6] = vid_colr[(i << 4) +  6]; }
                        if (flag & (1 <<  7)) { colors[ 7] = vid_colr[(i << 4) +  7]; }
                        if (flag & (1 <<  8)) { colors[ 8] = vid_colr[(i << 4) +  8]; }
                        if (flag & (1 <<  9)) { colors[ 9] = vid_colr[(i << 4) +  9]; }
                        if (flag & (1 << 10)) { colors[10] = vid_colr[(i << 4) + 10]; }
                        if (flag & (1 << 11)) { colors[11] = vid_colr[(i << 4) + 11]; }
                        if (flag & (1 << 12)) { colors[12] = vid_colr[(i << 4) + 12]; }
                        if (flag & (1 << 13)) { colors[13] = vid_colr[(i << 4) + 13]; }
                        if (flag & (1 << 14)) { colors[14] = vid_colr[(i << 4) + 14]; }
                        if (flag & (1 << 15)) { colors[15] = vid_colr[(i << 4) + 15]; }

                        *((uint32*)p) = (uint32)VecRaster16; p += 2;
                        *p++ = i - lastline;
                        *p++ = colors[ 0]; *p++ = colors[ 1]; *p++ = colors[ 2]; *p++ = colors[ 3];
                        *p++ = colors[ 4]; *p++ = colors[ 5]; *p++ = colors[ 6]; *p++ = colors[ 7];
                        *p++ = colors[ 8]; *p++ = colors[ 9]; *p++ = colors[10]; *p++ = colors[11];
                        *p++ = colors[12]; *p++ = colors[13]; *p++ = colors[14]; *p++ = colors[15];
                        lastline = i;
                        vid_colf[i] = 0;
                    }
                }
                *((uint32*)p) = (uint32)VecRasterEnd; p += 2;
                *p++ = 0;
                swapRasters = 1;
                EnableInterrupts();
            }
            else
            {
                for (short i=0; i<16; i++)
                    Setcolor(i, vid_col[i]);
            }
        }
#endif
        vid_flag = 0;
    }

    // blit
    register uint32* src = (uint32*) (membase + (vid_baseh << 16) + (vid_basem << 8));
    register uint32* dst = (uint32*) screenPtr[screenIdx];
    #define REPT10(x) x x x x x x x x x x
    #ifndef COLDFIRE
    if (hostcpu >= 40)
    { 
        // 68040+
        register int16 count = 19;
        __asm__ volatile (\
            "_l0%=:\n\t"\
            REPT10(REPT10("move16 (%0)+,(%1)+\n\t"))\
            "dbra.w %2,_l0%=\n\t"\
            : : "a"(src), "a"(dst), "d"(count) : "cc","memory");
    }
    else
    {
        // 68000+
        __asm__ volatile (
            "movem.l d1-d6/a0-a4,-(sp)\n\t"
            "move.l %0,a3\n\t"\
            "move.l %1,a4\n\t"\
            "move.w #99,d6\n\t"\
            "_l0%=:\n\t"\
            REPT10("movem.l (a3)+,d1-d5/a0-a2\n\t" "movem.l d1-d5/a0-a2,(a4)\n\t" "lea 32(a4),a4\n\t")\
            "dbra.w d6,_l0%=\n\t"\
            "movem.l (sp)+,d1-d6/a0-a4\n\t"
        : : "d"(src), "d"(dst) : "cc","memory");
    }
    #else
    {
        // Coldfire
        register int16 count = 99;
        while (count) {
            REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;)
            REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;) REPT10(*dst++ = *src++;)
            count--;
        }
    }
    #endif





    // present
    if (vdo < 3) {
        Setscreen(-1, screenPtr[screenIdx], -1);
    } else {
        VsetScreen(-1, screenPtr[screenIdx], -1, -1);
    }
    screenIdx = (screenIdx + 1) & 1;
    if (opt_framepacing) {
        WaitVBL();
    }
}

void HostSound() {
}


void VecKeyboard(void) {
    // This function must restore all clobbered registers and return with RTS
    // d0 contains the keyboard packet
    __asm__ volatile(
    "move.l d0,-(sp)\n\t"\
    "move.l d1,-(sp)\n\t"\
    "move.l a0,-(sp)\n\t"\
    "move.b d0,d1\n\t"\
    "move.l #_keys,a0\n\t"\
    "and.l  #0x7f,d1\n\t"\
    "move.b #1,(a0)\n\t"\
    "add.l  d1,a0\n\t"\
    "btst.b #7,d0\n\t"\
    "seq    d0\n\t"\
    "move.b d0,(a0)\n\t"\
    "move.l (sp)+,a0\n\t"\
    "move.l (sp)+,d1\n\t"\
    "move.l (sp)+,d0\n\t"\
    : : : "cc","memory" );
}

void VecMouse(void* p) {
    // This function must restore all clobbered registers and return with RTS
    // a0 points to the IKBD packet
    __asm__ volatile(
    "move.l d0,-(sp)\n\t"\
    "move.b (a0),d0\n\t"\
    "and.l  #3,d0\n\t"\
    "move.b d0,%0\n\t"\
    "move.b 1(a0),d0\n\t"\
    "extb.l d0\n\t"\
    "add.l  d0,%1\n\t"\
    "move.b 2(a0),d0\n\t"\
    "extb.l d0\n\t"\
    "add.l  d0,%2\n\t"\
    "move.l (sp)+,d0\n\t"\
    : : "m"(mouse.buttons),"m"(mouse.x),"m"(mouse.y) : "cc","memory");
}

void VecJoystick(void* p)
{
    // This function must restore all clobbered registers and return with RTS
    // a0 points to the IKBD packet
    __asm__ volatile(
    "move.l a1,-(sp)\n\t"\
    "move.l #_joys,a1\n\t"\
    "move.b 1(a0),0(a1)\n\t"\
    "move.b 2(a0),1(a1)\n\t"\
    "move.l (sp)+,a1\n\t"\
    : : : "cc","memory");
}


void HostEvents() {

    static short oldjoymask = 0;
    if (joymask != oldjoymask) {
        Bconout(4, (joymask & 1) ? 0x14 : 0x08);
        oldjoymask = joymask;
    }

    // joystick
    //DBG("joys %02x %02x", joys[0], joys[1]);
    for (short i=0; i<2; i++)
    {
        // legacy joystick
        uint8 status = joys[i];
        // extended vampire gamepad buttons
        if (vampire) {
            #define V4JOY_CONNECTED     0x0001
            #define V4JOY_BUTTON_A      0x0002
            #define V4JOY_BUTTON_B      0x0004
            #define V4JOY_BUTTON_X      0x0008
            #define V4JOY_BUTTON_Y      0x0010
            #define V4JOY_BUTTON_START  0x0400
            #define V4JOY_BUTTON_BACK   0x0200
            #define V4JOY_PAD_UP        0x1000
            #define V4JOY_PAD_DOWN      0x2000
            #define V4JOY_PAD_LEFT      0x4000
            #define V4JOY_PAD_RIGHT     0x8000

            uint16 now = Vampire_GetUsbJoystick(1 - i);
            if (now & V4JOY_CONNECTED) {
                static uint16 vjoy[2] = {0,0};
                uint16 old = vjoy[i];
                vjoy[i] = now;
                status |= (uint8) ((now & V4JOY_BUTTON_X) >> 3);        // x     -> up
                if ((now & (V4JOY_BUTTON_START|V4JOY_BUTTON_BACK|V4JOY_BUTTON_Y)) !=
                    (old & (V4JOY_BUTTON_START|V4JOY_BUTTON_BACK|V4JOY_BUTTON_Y)))
                {
                    now >>= 4;
                    joykeys[0x39] = (now & (V4JOY_BUTTON_Y>>4));        // y     -> space
                    joykeys[0x01] = (now & (V4JOY_BUTTON_BACK>>4));     // back  -> esc
                    joykeys[0x1c] = (now & (V4JOY_BUTTON_START>>4));    // start -> return
                    keys[0] = 1;
                }
            }
        }
        IkbdJoystickEvent(i, status);
    }

    // ctrl+alt?
    short hostCtrl = keys[0x1d] | keys[0x38];
    
    // keyboard
    if (keys[0]) {
        for (short i=1; i<128; i++) {
            uint8 key = keys[i] | joykeys[i];
            if (key != keys[i+128]) {
                keys[i+128] = key;
                if (key) {
                    IkbdKeyPress(i);
                    if (hostCtrl) {
                        if (i >= 0x02 && i <= 0x0a) {
                            // ctrl-alt-1 through 9
                            InsertDisk(i-1);
                        } else if (i == 0x14) {
                            // ctrl-alt-t
                            opt_framepacing = !opt_framepacing;
                        } else if (i == 0x20) {
                            // ctrl-alt-d
                        } else if ((i == 0x10) || (i == 0x01)) {
                            // ctrl-alt-esc / ctrl-alt-q
                            QuitEmulator();
                        }
                    }
                } else {
                    IkbdKeyRelease(i);
                }
                return;
            }
        }
        keys[0] = 0;
    }
}

short HostInit()
{
    // sound
    psgmode = vampire ? PSGMODE_XBIOS : PSGMODE_DIRECT;
    for (short i=0; i<16; i++) {
        oldSoundRegs[i] = Giaccess(0, i);
    }

    #if !DISABLE_GRAPHICS
    {
        // backup palette
        if (vdo >= 3) {
            VgetRGB(0, 256, oldPaletteExt);
        } else if (vdo == 2) {
            for (short i=0; i<256; i++) {
                oldPaletteExt[i] = EsetColor(i, 0);
                EsetColor(i, oldPaletteExt[i]);
            }
        }
        for (short i=0; i<16; i++) {
            oldPalette[i] = Setcolor(i, 0);
            Setcolor(i, oldPalette[i]);
        }

        // change screen mode
        oldPhysbase = Physbase();
        if (vampire && VAMPIRE_AMIGA_SCREEN)
        {
            oldRez = VsetMode(-1);
            Amiga_InitScreen();
        }
        else
        {
            // rasters
            rasterIdx = 0;
            vid_rasters = 1;
            RasterBuffer[0] = (uint8*)AllocateMem(32 * 1024, 4, MEM_FAST);
            RasterBuffer[1] = RasterBuffer[0] + (16 * 1024);

            // screen
            const uint32 screensize = 320 * 200 / 2;
            uint32 mem = AllocateMem(screensize * 2, 256, MEM_CHIP);
            screenPtr[0] = (uint16*) mem;
            screenPtr[1] = (uint16*) (mem + screensize);
            WaitVBL();
            switch (vdo)
            {
                case 0:
                case 1:
                    oldRez = Getrez();
                    modevals[0] = 0;
                    modevals[1] = 1;
                    modevals[2] = 2;
                    Setscreen(-1, screenPtr[screenIdx], modevals[0]);
                    break;
                case 2:
                    oldRez = EgetShift();
                    modevals[0] = 0 << 0;
                    modevals[1] = 1 << 8;
                    modevals[2] = 2 << 8;
                    Setscreen(-1, screenPtr[screenIdx], -1);
                    EsetShift(modevals[0]);
                    break;
                case 3:
                    oldRez = VsetMode(-1);
                    modevals[0] = STMODES|PAL|BPS4|COL40|(oldRez&VGA);
                    modevals[1] = STMODES|PAL|BPS2|COL80|(oldRez&VGA);
                    modevals[2] = STMODES|PAL|BPS1|COL80|VERTFLAG|(oldRez&VGA);
                    VsetScreen(-1, screenPtr[screenIdx], -1, -1);
                    VsetMode(modevals[0]);
                    break;
            }
            screenIdx = (screenIdx + 1) & 1;
            WaitVBL();
        }
    }
    #endif

    // input
    {
        // mouse mode
        Bconout(4,0x08);

        // hook ikbd vectors
        DisableInterrupts();
        _KBDVECS* kbdv = Kbdvbase();
        void(**kb)(void) = (void(**)(void))(uint32)kbdv;
        oldMouseVector = kbdv->mousevec;
        oldJoystickVector = kbdv->joyvec;
        oldKeyboardVector = kb[-1];
        kb[-1] = VecKeyboard;
        kbdv->mousevec = VecMouse;
        kbdv->joyvec = VecJoystick;
        EnableInterrupts();
        memset(keys,    0,128*2);
        memset(joykeys, 0,128*1);
        memset(joys,    0,  2*2);
    }

    // interrupts
    {
        DisableInterrupts();
        oldVblQ = (uint32*) *((volatile uint32*)0x00000456);
        oldVblS = *((volatile uint16*)0x00000454);

        newVblS = oldVblS + 1;
        newVblQ = (uint32*) HostAlloc(newVblS * sizeof(uint32*), MEM_FAST);
        memset(newVblQ, 0, newVblS * sizeof(uint32*));
        memcpy(newVblQ, oldVblQ, oldVblS * sizeof(uint32*));

        for (short i=0; i<newVblS; i++) {
            if (newVblQ[i] == 0) {
                newVblQ[i] = (uint32) VecVbl;
                break;
            }
        }
    	*((volatile uint32*)0x00000456) = (uint32) newVblQ;
	    *((volatile uint16*)0x00000454) = newVblS;
        EnableInterrupts();
    }

    initstate = 1;
    return 1;
}

void HostExit()
{
    if (initstate != 0)
    {
        // sound
        for (short i=0; i<16; i++) {
            Giaccess(oldSoundRegs[i], (0x80 | i));
        }

        // interrupts and vectors
        DisableInterrupts();
    	*((volatile uint32*)0x00000456) = (uint32) oldVblQ;
	    *((volatile uint16*)0x00000454) = oldVblS;

        // ikbd vectors
        _KBDVECS* kbdv = Kbdvbase();
        void(**kb)(void) = (void(**)(void))(uint32)kbdv;
        kb[-1] = oldKeyboardVector;
        kbdv->mousevec = oldMouseVector;
        kbdv->joyvec = oldJoystickVector;
        EnableInterrupts();

        // mouse mode
        Bconout(4,0x08);

        // screen
        #if !DISABLE_GRAPHICS
        {
            if (vampire && VAMPIRE_AMIGA_SCREEN)
            {
                Amiga_ReleaseScreen();
            }

            WaitVBL();
            switch (vdo)
            {
                case 0:
                case 1:
                    Setscreen(-1, oldPhysbase, oldRez);
                    break;
                case 2:
                    Setscreen(-1, oldPhysbase, -1);
                    EsetShift(oldRez);
                    break;
                case 3:
                    VsetScreen(-1, oldPhysbase, -1, -1);
                    VsetMode(oldRez);
                    break;
            }

            // palette
            for (short i=0; i<16; i++) {
                Setcolor(i, oldPalette[i]);
            }
            if (vdo >= 3) {
                VsetRGB(0, 256, oldPaletteExt);
            } else if (vdo == 2) {
                for (short i=0; i<256; i++)
                    EsetColor(i, oldPaletteExt[i]);
            }
        }
        #endif
        initstate = 0;
    }
}

