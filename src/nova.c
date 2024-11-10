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
uint32* nova_cmpbuffer;
static uint32 framenum = 0;

extern int16 vdiHandlep;
static uint32 oldpalsize = 0;
static uint16 oldpal[256*3];

typedef struct
{
    uint16 st_width;
    uint16 st_height;
    uint16 st_bpp;
    uint16 nova_width;
    uint16 nova_height;
    uint16 nova_bpp;
    uint32 nova_offs;
    uint32 nova_pitch;
    uint32 nova_size;
    nova_resolution_t nova;
} screenmode_t;

screenmode_t screenmodes[3];
nova_resolution_t oldres;



static inline void Nova_SetNovaScreen(nova_resolution_t* res) {
    // set resolution
    __asm__ __volatile__ (
            "moveql	#0,%%d0\n\t"
            "movel	%0,%%a0\n\t"
            "movel	%1,%%a1\n\t"
            "jsr	%%a1@"
        : : "g"(res), "g"(nova_xcb->p_chres)
        : "d0", "d1", "d2", "a0", "a1", "cc", "memory"
    );

    // vsync
	__asm__ __volatile__ (
		"movel	%0,%%a0\n\t"
		"jsr	%%a0@"
		: : "g"(nova_xcb->p_vsync)
		: "d0", "d1", "d2", "a0", "a1", "cc", "memory"
		);
}

void Nova_SetAtariScreen(short shiftmode)
{
	nova_shiftmode = shiftmode;
    if (screenmodes[shiftmode].nova_size) {
        Nova_SetNovaScreen(&screenmodes[shiftmode].nova);
        memset(nova_physbase, 0, screenmodes[shiftmode].nova_size);
        memset(nova_cmpbuffer, 0, 64 * 1024);
	}
}

void Nova_Blit()
{
    // mode
    if (vid_shiftmode != nova_shiftmode) {
        Nova_SetAtariScreen(vid_shiftmode);
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
    screenmode_t* mode = &screenmodes[nova_shiftmode];
	register uint32* cmp = (uint32*) nova_cmpbuffer;
    register uint32* src = (uint32*) (membase + (vid_baseh << 16) + (vid_basem << 8));
	register uint32* dst = (uint32*) (nova_physbase + mode->nova_offs);

    if (nova_shiftmode == 0)
    {
        // ST-Low -> Nova 8bpp
        for (short y=199; y>=0; y--)
        {
            for (short x=19; x >= 0; x--)
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
            dst += (mode->nova_width - 320) >> 2;
        }
    }
    else if (nova_shiftmode == 1)
    {
        // ST-Medium -> Nova 8bpp
    }
    else if (nova_shiftmode == 2)
    {
        if (mode->nova_bpp == 1)
        {
            // ST-High -> Nova 1bpp
        }
        else
        {
            // ST-High -> Nova 8bpp
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


uint8 Nova_ChooseMode(nova_resolution_t* nova_res, screenmode_t* st_res) {
    uint16 nova_bpp_best = 2;
    uint16 nova_bpp_accept = 2;
    if (st_res->st_bpp == 1) {
        nova_bpp_best = 1;
    }
    if ((nova_res->mode != nova_bpp_best) && (nova_res->mode != nova_bpp_accept)) {
        return 0;
    }

    char accept = 0;
    uint16 w = nova_res->real_x + 1;
    uint16 h = nova_res->real_y + 1;
    if ((w >= st_res->st_width) && (h >= st_res->st_height) && ((nova_res->mode == nova_bpp_best) || (nova_res->mode == nova_bpp_accept))) {
        if (st_res->nova_size == 0) {
            // accept if we didn't have any mode selected
            accept = 1;
        }
        else if ((nova_bpp_best != nova_bpp_accept) && (st_res->nova_bpp == nova_bpp_best) && (nova_res->mode == nova_bpp_accept)) {
            // ignore if bpp option is worse than what we have
            accept = 0;
        }
        else if (w < st_res->nova_width) {
            // accept if width is closer
            accept = 1;
        }
        else if (h < st_res->nova_height) {
            // accept if height is closer
            accept = 1;
        }
    }

    if (!accept) {
        return 0;
    }

    memcpy(&st_res->nova, nova_res, sizeof(nova_resolution_t));
    switch (nova_res->mode) {
        case 1:
            st_res->nova_bpp = 1;
            break;
        default:
            st_res->nova_bpp = 8;
            break;
    }
    st_res->nova_width = w;
    st_res->nova_height = h;
    st_res->nova_pitch = (st_res->nova_width * st_res->nova_bpp) >> 3;
    st_res->nova_offs = (((st_res->nova_height - st_res->st_height) >> 1) * st_res->nova_pitch) + (((st_res->nova_width - st_res->st_width) >> 1) & ~3UL);
    st_res->nova_size = st_res->nova_pitch * st_res->nova_height;
    return 1;
}

void Nova_LoadModes()
{
	char filename[32];
    strcpy(filename, "c:\\auto\\sta_vdi.bib");
    filename[0] = 'a'+ *((volatile unsigned short *)0x446);

	int fhandle = open(filename, 0);
	if (fhandle < 0) {
		DBG("Failed opening sta_vdi.bib");
		return;
	}
	int fsize = lseek(fhandle, 0, SEEK_END);
	lseek(fhandle, 0, SEEK_SET);

	for (int fpos = 0, m = 0; (fpos + sizeof(nova_resolution_t)) <= fsize; fpos += sizeof(nova_resolution_t), m++)
	{
		nova_resolution_t res;
		read(fhandle, &res, sizeof(nova_resolution_t));
		res.dummy1 = 0;
        if (m == nova_xcb->resolution) {
            DBG("Desk res %d : %d %d", m, res.real_x+1, res.real_y+1);
            memcpy(&oldres, &res, sizeof(nova_resolution_t));
        }
        Nova_ChooseMode(&res, &screenmodes[0]);
        Nova_ChooseMode(&res, &screenmodes[1]);
        Nova_ChooseMode(&res, &screenmodes[2]);
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

    // prepare st modes
    memset(screenmodes, 0, sizeof(screenmode_t) * 3);
    screenmodes[0].st_width = 320;
    screenmodes[0].st_height = 200;
    screenmodes[0].st_bpp = 4;
    screenmodes[1].st_width = 640;
    screenmodes[1].st_height = 200;
    screenmodes[1].st_bpp = 2;
    screenmodes[2].st_height = 640;
    screenmodes[2].st_height = 400;
    screenmodes[2].st_bpp = 1;

	// load and match against nova modes
	Nova_LoadModes();
    for (int i=0; i<3; i++) {
        DBG("STMODE %d : %d %d %d", i, screenmodes[i].nova_width, screenmodes[i].nova_height, screenmodes[i].nova_bpp);
    }
   
    // backup palette
    oldpalsize = ((vdiHandlep >= 0) && (nova_xcb->planes <= 8)) ? (1 << nova_xcb->planes) : 0;
    for (int16 i = 0; i < oldpalsize; i++) {
        vq_color(vdiHandlep, i, 1, &oldpal[i * 3]);
    }

    // set screenmode
	Nova_SetAtariScreen(0);
}

void Nova_ReleaseScreen()
{
    // restore screenmode
    Nova_SetNovaScreen(&oldres);

    // restore palette
    extern int16 vdiHandlep;
    for (int16 i = 0; i < oldpalsize; i++) {
        vs_color(vdiHandlep, i, &oldpal[i * 3]);
    }
}

