#include "config.h"
#include "68000.h"
#include "mem.h"
#include "st.h"
#include "nova.h"

extern uint16* oldPhysbase;
extern uint16* screenPtr[2];
extern short screenIdx;
short nova_shiftmode;
uint32 nova_physbase;
nova_xcb_t *nova_xcb;
nova_resolution_t nova_modes[3];	// low, med, high
uint32* nova_cmpbuffer;
static uint32 framenum = 0;

void Nova_SetScreen(short mode)
{
	nova_shiftmode = mode;
	if (nova_modes[mode].mode == 2)
	{
		__asm__ __volatile__ (
				"moveql	#0,%%d0\n\t"
				"movel	%0,%%a0\n\t"
				"movel	%1,%%a1\n\t"
				"jsr	%%a1@"
			: : "g"(&nova_modes[mode]), "g"(nova_xcb->p_chres)
			: "d0", "d1", "d2", "a0", "a1", "cc", "memory"
		);
	}
}



void Nova_Blit()
{
    // mode
    if (vid_shiftmode != nova_shiftmode) {
        Nova_SetScreen(vid_shiftmode);
    }

    // wait vbl
    if (opt_framepacing) {
        while(1) {
            uint32 curframe = *((volatile uint32*)0x00000462);
            if (curframe != framenum) {
                framenum = curframe;
                break;
            }
        }
    }

	// screen
	register uint32* cmp = (uint32*) nova_cmpbuffer;
    register uint32* src = (uint32*) (membase + (vid_baseh << 16) + (vid_basem << 8));
	register uint32* dst = (uint32*) nova_physbase;
	for (short y=0; y<200; y++)
	{
		for (short x=0; x<20; x++)
		{
			if ((src[0] == cmp[0]) && (src[1] == cmp[1])) {
				src += 2;
				cmp += 2;
				dst += 4;
			}
			else {
				register uint32 l0 = *src++; *cmp++ = l0;
				register uint32 l1 = *src++; *cmp++ = l1;

				for (short j=3; j>=0; j--)
				{
					register uint32 c0123 = 0;
					register uint32 mask = 0x01000000;
					// pixel0
					c0123 |= (l0 >>  7) & mask; mask <<= 1;
					c0123 |= (l0 << 10) & mask; mask <<= 1;
					c0123 |= (l1 >>  5) & mask; mask <<= 1;
					c0123 |= (l1 << 12) & mask; mask >>= 11;
					// pixel1
					c0123 |= (l0 >> 14) & mask; mask <<= 1;
					c0123 |= (l0 <<  3) & mask; mask <<= 1;
					c0123 |= (l1 >> 12) & mask; mask <<= 1;
					c0123 |= (l1 <<  5) & mask; mask >>= 11;
					// pixel2
					c0123 |= (l0 >> 21) & mask; mask <<= 1;
					c0123 |= (l0 >>  4) & mask; mask <<= 1;
					c0123 |= (l1 >> 19) & mask; mask <<= 1;
					c0123 |= (l1 >>  2) & mask; mask >>= 11;
					// pixel3
					c0123 |= (l0 >> 28) & mask; mask <<= 1;
					c0123 |= (l0 >> 11) & mask; mask <<= 1;
					c0123 |= (l1 >> 26) & mask; mask <<= 1;
					c0123 |= (l1 >>  9) & mask;
					// next chunk
					l0 <<= 4;
					l1 <<= 4;
					*dst++ = c0123;
				}
			}
		}
	}

	// palette
    if ((vid_flag & VIDFLAG_PALETTE) && (vid_shiftmode != 2))
	{
		for (int i = 0; i < 16; i++)
		{
			uint16 c = vid_col[i];
			unsigned char color[3];
			color[0] = (c >> 3) & 0xE0;
			color[1] = (c << 1) & 0xE0;
			color[2] = (c << 5) & 0xE0;
			__asm__ __volatile__ (
					"movel	%0,%%d0\n\t"
					"movel	%1,%%a0\n\t"
					"movel	%2,%%a1\n\t"
					"jsr	%%a1@"
				: : "g"(i), "g"(color), "g"(nova_xcb->p_setcol)
				: "d0", "d1", "d2", "a0", "a1", "cc", "memory"
			);
		}
	}

    // flip
    screenIdx = (screenIdx + 1) & 1;
}


void Nova_LoadModes()
{
	char filename[32];
    strcpy(filename, "c:\\auto\\sta_vdi.bib");
    filename[0] = 'a'+ *((volatile unsigned short *)0x446);
	memset(nova_modes, 0, 3 * sizeof(nova_resolution_t));

	int fhandle = open(filename, 0);
	if (fhandle < 0) {
		DBG("Failed opening sta_vdi.bib");
		return;
	}
	int fsize = lseek(fhandle, 0, SEEK_END);
	lseek(fhandle, 0, SEEK_SET);

	for (int fpos = 0; (fpos + sizeof(nova_resolution_t)) <= fsize; fpos += sizeof(nova_resolution_t))
	{
		nova_resolution_t res;
		read(fhandle, &res, sizeof(nova_resolution_t));
		res.dummy1 = 0;
		//DBG("mode: %s : %d,%d,%d", res.name, res.mode, res.real_x, res.real_y);
		if (res.mode == 2)
		{
			if (res.real_x == 319 && res.real_y == 199)
				memcpy(&nova_modes[0], &res, sizeof(nova_resolution_t));
			else if (res.real_x == 639 && res.real_y == 199)
				memcpy(&nova_modes[1], &res, sizeof(nova_resolution_t));
			else if (res.real_x == 639 && res.real_y == 399)
				memcpy(&nova_modes[2], &res, sizeof(nova_resolution_t));
		}
	}

	close(fhandle);
}



void Nova_InitScreen()
{
    nova_shiftmode = 0;
    screenIdx = 0;
//    screenIdx = (screenIdx + 1) & 1;

	nova_xcb = 0;
	Getcookie('NOVA', &nova_xcb);
	if (nova_xcb == 0)
		return;

	nova_cmpbuffer = (uint32*) AllocateMem(64 * 1024, 16, MEM_FAST);
	nova_physbase = (uint32) oldPhysbase;
	memset((void*)nova_physbase, 0, 320*200/2);

	// find modes
	Nova_LoadModes();
	Nova_SetScreen(0);
}


void Nova_ReleaseScreen()
{
}

