#include "config.h"
#include "68000.h"
#include "mem.h"
#include "st.h"

#if !defined(EXCLUDE_VAMPIRE)

#define DMACONR         *(volatile uint16*)0xdff002
#define VPOSR           *(volatile uint32*)0xdff004
#define INTENAR         *(volatile uint16*)0xdff01c
#define BPLCON0         *(volatile uint16*)0xdff100
#define BPLCON1         *(volatile uint16*)0xdff102
#define BPLCON2         *(volatile uint16*)0xdff104
#define BPLCON3         *(volatile uint16*)0xdff106
#define BPL1MOD         *(volatile uint16*)0xdff108
#define BPL2MOD         *(volatile uint16*)0xdff10a
#define COLOR00         *(volatile uint16*)0xdff180
#define DIWSTRT         *(volatile uint16*)0xdff08e
#define DIWSTOP         *(volatile uint16*)0xdff090
#define DDFSTRT         *(volatile uint16*)0xdff092
#define DDFSTOP         *(volatile uint16*)0xdff094
#define DMACON          *(volatile uint16*)0xdff096
#define COP1LCH         *(uint16* volatile*)0xdff080
#define COPJMP1         *(volatile uint16*)0xdff088
#define FMODE           *(volatile uint16*)0xdff1fc
#define SAGA_GFXMODE    *(volatile uint16*)0xdff1f4
#define COLOR00         *(volatile uint16*)0xdff180
#define COLOR01         *(volatile uint16*)0xdff182

#define DMAEN    		(1U << 9)
#define BPLEN    		(1U << 8)
#define COPEN    		(1U << 7)

extern uint16* screenPtr[2];
extern short screenIdx;
extern short mach_vampire;

uint16 oldDMACON;
uint16 oldINTENA;
uint16  vstart, vstop;
uint16* copperPtr[2];
short vampire_shiftmode;

uint16 Vampire_GetUsbJoystick(unsigned char idx)
{
    return *((volatile uint16*)(0xdff220+(idx<<1)));
}

void Vampire_SetScreen(short mode) {
    vampire_shiftmode = mode;

    uint16 ddfstrt = 0x0038;
    uint16 hstart = 0x81;
    if (mach_vampire) {
        ddfstrt -= 8;
        hstart -= 16;
    }

    uint16 bplcon0;
    uint16 bpl1mod;

    switch (mode)
    {
        default:
        case 0:
        {
            for (int i=0; i<2; i++) {
                uint16* copperlist = copperPtr[i];
                uint32 p0 = ((uint32)screenPtr[i]) + 0;
                uint32 p1 = ((uint32)screenPtr[i]) + 8000;
                uint32 p2 = ((uint32)screenPtr[i]) + 16000;
                uint32 p3 = ((uint32)screenPtr[i]) + 24000;
                copperlist[ 3] = (uint16)(p0 >> 16);
                copperlist[ 5] = (uint16)(p0 & 0xFFFF);
                copperlist[ 7] = (uint16)(p1 >> 16);
                copperlist[ 9] = (uint16)(p1 & 0xFFFF);
                copperlist[11] = (uint16)(p2 >> 16);
                copperlist[13] = (uint16)(p2 & 0xFFFF);
                copperlist[15] = (uint16)(p3 >> 16);
                copperlist[17] = (uint16)(p3 & 0xFFFF);
            }
            bplcon0 = 0x0200 | (4 << 12);
            bpl1mod = 0;
        }
        break;

        case 1:
        {
            for (int i=0; i<2; i++) {
                uint16* copperlist = copperPtr[i];
                uint32 p0 = ((uint32)screenPtr[i]) + 0;
                uint32 p1 = ((uint32)screenPtr[i]) + 16000;
                copperlist[ 3] = (uint16)(p0 >> 16);
                copperlist[ 5] = (uint16)(p0 & 0xFFFF);
                copperlist[ 7] = (uint16)(p1 >> 16);
                copperlist[ 9] = (uint16)(p1 & 0xFFFF);
            }
            ddfstrt += 4;
            bplcon0 = 0x8200 | (2 << 12);
            bpl1mod = 0;
        }
        break;

        case 2:
        {
            for (int i=0; i<2; i++) {
                uint16* copperlist = copperPtr[i];
                uint32 p0 = ((uint32)screenPtr[i]) + 0;
                copperlist[ 3] = (uint16)(p0 >> 16);
                copperlist[ 5] = (uint16)(p0 & 0xFFFF);
            }
            ddfstrt += 4;
            bplcon0 = 0x8200 | 0x0004 | (1 << 12);
            bpl1mod = (640 / 8);
            COLOR00 = 0x0fff;
            COLOR01 = 0x0000;
        }
        break;
    }

    uint16 fetches = (320 / 2 / 8) - 1;
    uint16 ddfstop = ddfstrt + (fetches * 8);
    uint16 hstop = hstart + 320;
    hstop = (uint8)(((int16)hstop) - 0x100);
    vstart = 44 + ((256 - 200) / 2);
    vstop = vstart + 200;
    vstop = (uint8)(((int16)vstop) - 0x100);

    BPLCON0 = bplcon0;
    BPL1MOD = bpl1mod;
    DDFSTRT = ddfstrt;
    DDFSTOP = ddfstop;
    DIWSTRT = (((vstart << 8) & 0xFF00) | (hstart & 0x00FF));
    DIWSTOP = (((vstop  << 8) & 0xFF00) | (hstop  & 0x00FF));
}

void Vampire_Blit()
{
    // mode
    if (vid_shiftmode != vampire_shiftmode) {
        Vampire_SetScreen(vid_shiftmode);
    }

    // screen
    #define REPT10(x) x x x x x x x x x x
    switch (vampire_shiftmode)
    {
        default:
        case 0:
        {
            register uint16* src = (uint16*) (membase + (vid_baseh << 16) + (vid_basem << 8));
            register uint16* dst = (uint16*) screenPtr[screenIdx];
            register int16 count = 99;
            while(count) {
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; register uint16 s2 = *src++; register uint16 s3 = *src++; *(dst) = s0; *(dst+4000) = s1; *(dst+8000) = s2; *(dst + 12000) = s3; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; register uint16 s2 = *src++; register uint16 s3 = *src++; *(dst) = s0; *(dst+4000) = s1; *(dst+8000) = s2; *(dst + 12000) = s3; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; register uint16 s2 = *src++; register uint16 s3 = *src++; *(dst) = s0; *(dst+4000) = s1; *(dst+8000) = s2; *(dst + 12000) = s3; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; register uint16 s2 = *src++; register uint16 s3 = *src++; *(dst) = s0; *(dst+4000) = s1; *(dst+8000) = s2; *(dst + 12000) = s3; dst++; })
                count--;
            }
        }
        break;

        case 1:
        {
            register uint16* src = (uint16*) (membase + (vid_baseh << 16) + (vid_basem << 8));
            register uint16* dst = (uint16*) screenPtr[screenIdx];
            register int16 count = 199;
            while(count) {
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; *(dst) = s0; *(dst+8000) = s1; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; *(dst) = s0; *(dst+8000) = s1; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; *(dst) = s0; *(dst+8000) = s1; dst++; })
                REPT10({register uint16 s0 = *src++; register uint16 s1 = *src++; *(dst) = s0; *(dst+8000) = s1; dst++; })
                count--;
            }
        }
        break;

        case 2:
        {
            register uint32* src = (uint32*) (membase + (vid_baseh << 16) + (vid_basem << 8));
            register uint32* dst = (uint32*) screenPtr[screenIdx];
            register int16 count = 49;
            while(count) {
                REPT10({*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = *src++; })
                REPT10({*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = *src++; })
                REPT10({*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = *src++; })
                REPT10({*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = *src++; })
                count--;
            }
        }
        break;
    }

    // palette
    register uint16* cop = copperPtr[screenIdx] + 18;
    if ((vid_flag & VIDFLAG_PALETTE) && (vid_shiftmode != 2))
    {
        if (vid_rasters)
        {
            #define COPPER_WRITE_RASTER(l,x) if (flag & (1<<x)) { *cop++ = (0x0180 + (x*2)); *cop++ = vid_colr[(l << 4) + x] << 1; }
            {
                uint16 flag = vid_colf[0];
                if (flag) {
                    COPPER_WRITE_RASTER(0,0);  COPPER_WRITE_RASTER(0,1);  COPPER_WRITE_RASTER(0,2);  COPPER_WRITE_RASTER(0,3);
                    COPPER_WRITE_RASTER(0,4);  COPPER_WRITE_RASTER(0,5);  COPPER_WRITE_RASTER(0,6);  COPPER_WRITE_RASTER(0,7);
                    COPPER_WRITE_RASTER(0,8);  COPPER_WRITE_RASTER(0,9);  COPPER_WRITE_RASTER(0,10); COPPER_WRITE_RASTER(0,11);
                    COPPER_WRITE_RASTER(0,12); COPPER_WRITE_RASTER(0,13); COPPER_WRITE_RASTER(0,14); COPPER_WRITE_RASTER(0,15);
                    vid_colf[0] = 0;
                }
            }
            for (short i=1; i<200; i++) {
                uint16 flag = vid_colf[i];
                if (flag) {
                    *cop++ = (((vstart + i) & 0xFF) << 8) + 0x07;
                    *cop++ = 0xfffe;
                    COPPER_WRITE_RASTER(i,0);  COPPER_WRITE_RASTER(i,1);  COPPER_WRITE_RASTER(i,2);  COPPER_WRITE_RASTER(i,3);
                    COPPER_WRITE_RASTER(i,4);  COPPER_WRITE_RASTER(i,5);  COPPER_WRITE_RASTER(i,6);  COPPER_WRITE_RASTER(i,7);
                    COPPER_WRITE_RASTER(i,8);  COPPER_WRITE_RASTER(i,9);  COPPER_WRITE_RASTER(i,10); COPPER_WRITE_RASTER(i,11);
                    COPPER_WRITE_RASTER(i,12); COPPER_WRITE_RASTER(i,13); COPPER_WRITE_RASTER(i,14); COPPER_WRITE_RASTER(i,15);
                    vid_colf[i] = 0;
                }
            }
        }
        else
        {
            #define COPPER_WRITE_PAL(x) { *cop++ = (0x0180 + (x*2)); *cop++ = vid_col[x] << 1; }
            COPPER_WRITE_PAL(0);  COPPER_WRITE_PAL(1);  COPPER_WRITE_PAL(2);  COPPER_WRITE_PAL(3);
            COPPER_WRITE_PAL(4);  COPPER_WRITE_PAL(5);  COPPER_WRITE_PAL(6);  COPPER_WRITE_PAL(7);
            COPPER_WRITE_PAL(8);  COPPER_WRITE_PAL(9);  COPPER_WRITE_PAL(10); COPPER_WRITE_PAL(11);
            COPPER_WRITE_PAL(12); COPPER_WRITE_PAL(13); COPPER_WRITE_PAL(14); COPPER_WRITE_PAL(15);
        }
    }
    *cop++ = 0xffff;
    *cop++ = 0xfffe;

    // flip
    COP1LCH = copperPtr[screenIdx];
    screenIdx = (screenIdx + 1) & 1;
    if (opt_framepacing) {
        while(((*((volatile uint32*)0xdff004)) & 0x1ff00) < ((vstart + 200) << 8)) {}
    }
}


void Vampire_InitScreen()
{
    oldDMACON = DMACONR;
    oldINTENA = INTENAR;
    vid_rasters = 1;

    vampire_shiftmode = 0;
    screenIdx = 0;

    const uint32 screensize = (320 * 200 / 2);
    const uint32 coppersize = (16 * 1024);
    const uint32 totalsize = (screensize * 2) + (coppersize * 2);
    uint8* mem = (uint8*) AllocateMem(totalsize, 256, MEM_CHIP);
    screenPtr[0] = (uint16*) mem;
    screenPtr[1] = (uint16*) (mem + screensize);
    copperPtr[0] = (uint16*) (mem + screensize + screensize);
    copperPtr[1] = (uint16*) (mem + screensize + screensize + coppersize);
    for (int i=0; i<2; i++) {
        uint16* copperlist = copperPtr[i];
        uint32 p0 = ((uint32)screenPtr[i]) + 0;
        uint32 p1 = ((uint32)screenPtr[i]) + 8000;
        uint32 p2 = ((uint32)screenPtr[i]) + 16000;
        uint32 p3 = ((uint32)screenPtr[i]) + 24000;
        *copperlist++ = 0x0a01;
        *copperlist++ = 0xff00;
        *copperlist++ = 0x0e0; *copperlist++ = (uint16)(p0 >> 16);
        *copperlist++ = 0x0e2; *copperlist++ = (uint16)(p0 & 0xFFFF);
        *copperlist++ = 0x0e4; *copperlist++ = (uint16)(p1 >> 16);
        *copperlist++ = 0x0e6; *copperlist++ = (uint16)(p1 & 0xFFFF);
        *copperlist++ = 0x0e8; *copperlist++ = (uint16)(p2 >> 16);
        *copperlist++ = 0x0ea; *copperlist++ = (uint16)(p2 & 0xFFFF);
        *copperlist++ = 0x0ec; *copperlist++ = (uint16)(p3 >> 16);
        *copperlist++ = 0x0ee; *copperlist++ = (uint16)(p3 & 0xFFFF);
        *copperlist++ = 0xffff; // 18
        *copperlist++ = 0xfffe;        
    }

    uint16 ddfstrt = 0x0038;
    if (mach_vampire)
        ddfstrt -= 8;
    uint16 fetches = (320 / 2 / 8) - 1;
    uint16 ddfstop = ddfstrt + (fetches * 8);

    uint16 hstart = 0x81;
    if (mach_vampire)
        hstart -= 16;
    uint16 hstop = hstart + 320;
    hstop = (uint8)(((int16)hstop) - 0x100);

    vstart = 44 + ((256 - 200) / 2);
    vstop = vstart + 200;
    vstop = (uint8)(((int16)vstop) - 0x100);

    // disable saga chunky buffer
    if (mach_vampire) {
        SAGA_GFXMODE = 0;
    }

    //FMODE = 0;
    BPLCON0 = 0x4200;
    BPLCON1 = 0;
    BPLCON2 = 0;
    BPLCON3 = 0;
    BPL1MOD = 0;
    BPL2MOD = 0;
    DDFSTRT = ddfstrt;
    DDFSTOP = ddfstop;
    DIWSTRT = (((vstart << 8) & 0xFF00) | (hstart & 0x00FF));
    DIWSTOP = (((vstop  << 8) & 0xFF00) | (hstop  & 0x00FF));
    COP1LCH = copperPtr[screenIdx];
    COPJMP1 = 0;
    DMACON = (1 << 15) | COPEN | BPLEN | DMAEN;
    screenIdx = (screenIdx + 1) & 1;
}


void Vampire_ReleaseScreen() {
    // bitplanes and copper off
    BPLCON0 = 0;
    DMACON = COPEN | BPLEN;
}

#endif // EXCLUDE_VAMPIRE
