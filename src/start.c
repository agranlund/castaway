#include "config.h"
#include <mint/mintbind.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>
#include <mint/falcon.h>
#include "gem.h"

extern void (*const jmp_table[8192*8])();

extern short hostcpu;
extern short mach_vampire;
extern short mach_raven;
extern uint16 vidcaps;
extern uint16 sndcaps;
int16 appId;
int16 aesVersion;
int16 vdiHandlep;

int g_args;
char** g_argv;


int supermain()
{
    extern void Opc0c0();
    OSHEADER* osheader = *((OSHEADER**)0x4f2);
    DBG("osheader = %08x", (uint32) osheader);
    BASEPAGE* bp = (BASEPAGE*) *(osheader->p_run);
    DBG("basepage:   %08x", (uint32) bp);
    DBG("text:       %08x (%08x)", (uint32) bp->p_tbase, (uint32) bp->p_tlen);
    DBG("data:       %08x (%08x)", (uint32) bp->p_dbase, (uint32) bp->p_dlen);
    DBG("bss:        %08x (%08x)", (uint32) bp->p_bbase, (uint32) bp->p_blen);
    DBG("jmp_table:  %08x", (uint32) jmp_table);
    DBG("Opc0c0:     %08x", (uint32) Opc0c0);

    uint32 cookie, nova;
	uint16 vdo = 0; uint16 snd = 0; uint16 mch = 0;
    cookie = 0x0000; Getcookie('_CPU', &cookie); hostcpu = (uint16) cookie;
    cookie = 0x0000; Getcookie('_SND', &cookie); snd = (uint16) cookie;
    cookie = 0xFFFF; Getcookie('_VDO', &cookie); vdo = (cookie >> 16);
    cookie = 0xFFFF; Getcookie('_MCH', &cookie); mch = (cookie >> 16);
    cookie = 0x0000; Getcookie('RAVN', &cookie); mach_raven = (cookie == 0) ? 0 : 1;
	nova = 0; Getcookie('NOVA', &nova);

#ifndef COLDFIRE
	// todo: is there a better way to identify Vampire?
	mach_vampire = (mch == 6) ? 1 : 0;
#else
	mach_vampire = 0;
#endif

	// video mode
	switch (vdo) {
		case 0:
		case 1:
			vidcaps = VIDEO_ST;
			break;
		case 2:
			vidcaps = VIDEO_TT;
			break;
		case 3:
			vidcaps = VIDEO_FALCON;
			break;
		default:
			vidcaps = VIDEO_OFF;
	}
	if (mach_vampire) {
		vidcaps = VIDEO_VAMPIRE;
	}
	if (nova) {
		vidcaps = VIDEO_NOVA;
	}

	// sound mode
	sndcaps = SOUND_OFF;
	if (snd & 1)
		sndcaps |= SOUND_PSG;

	DBG("vidcaps = %d", vidcaps);
	DBG("sndcaps = %d", sndcaps);


    if (hostcpu < 20) { HALT("This program requires 68020+"); return -1; }
    #if defined(AC68080)
    if (hostcpu != 80) { HALT("This program requires 68080"); return -1; }
    #endif
    #if defined(VAMPIRE)
    if (!mach_vampire) { HALT("This program requires Vampire"); return -1; }
    #endif

    StartEmulator(g_args, g_argv);
	DBG("HostExit");
    HostExit();
	DBG("Main");
    return 0;
}


#define C_UNION(x) { (uint32) x }
OBJECT rs_menu[] = {
	{ -1,  1,  4, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      0,0,    21,19 },
	{  4,  2,  2, G_BOX,    OF_NONE, OS_NORMAL,     C_UNION(0x1100L),                   0,0,    21,19 },
	{  1,  3,  3, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      2,0,    1,19 },
	{  2, -1, -1, G_TITLE,  OF_NONE, OS_NORMAL,     C_UNION(" "),                       0,0,    1,19 },
	{  0,  5,  5, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      0,23,   21,19 },
	{  4,  6, 13, G_BOX,    OF_NONE, OS_NORMAL,     C_UNION(0xFF1100L),                 2,0,    21,8 },
	{  7, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  About"),                 0,0,    21,1 },
	{  8, -1, -1, G_STRING, OF_NONE, OS_DISABLED,   C_UNION("---------------------"),   0,1,    21,1 },
	{  9, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  Desk Accessory 1"),      0,2,    21,1 },
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  Desk Accessory 2"),      0,3,    21,1 },
	{ 11, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  Desk Accessory 3"),      0,4,    21,1 },
	{ 12, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  Desk Accessory 4"),      0,5,    21,1 },
	{ 13, -1, -1, G_STRING, OF_NONE, OS_NORMAL,     C_UNION("  Desk Accessory 5"),      0,6,    21,1 },
	{  5, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL,   C_UNION("  Desk Accessory 6"),      0,7,    21,1 }
};

int main(int args, char* argv[])
{
#ifdef DEBUGBUILD
    Fforce(1, -2);
#endif

	aes_global[0] = 0;
	appId = appl_init();
	aesVersion = aes_global[0];
    vdiHandlep = -1;
    int16 vdiHandlev = -1;
    int16 wndScreen = -1;
    int16 width, height;
    int16 oldpalsize = 0;
    int16 black[3] = {0,0,0};
    int16 desk_x, desk_y, desk_w, desk_h;

	if ((appId >= 0) && (aesVersion > 0))
	{
        int16 dummy; int16 work_in[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2}; int16 work_out[57];

        vdiHandlep = graf_handle(&dummy, &dummy, &dummy, &dummy);
        if (vdiHandlep >= 0) {
            vdiHandlev = vdiHandlep;
            if (vdiHandlev >= 0) {
                v_opnvwk(work_in, &vdiHandlev, work_out);
                width = work_out[0] + 1;
                height = work_out[1] + 1;
                wndScreen = wind_create(0, 0, 0, width, height);
                wind_open(wndScreen, 0, 0, width, height);
            }
        }

        menu_bar(rs_menu, 1);
        wind_update(BEG_UPDATE);
        wind_update(BEG_MCTRL);
        graf_mouse(M_OFF, 0);
    }

    g_args = args;
    g_argv = argv;
    Supexec(supermain);
    
    if (appId >= 0) {
        if (aesVersion > 0) {
            if (wndScreen >= 0) {
                wind_close(wndScreen);
            }
            if (vdiHandlev >= 0) {
                v_clsvwk(vdiHandlev);
            }
            menu_bar(rs_menu, 0);
            graf_mouse(M_ON, 0);
            graf_mouse(ARROW, 0);
            wind_update(END_MCTRL);
            wind_update(END_UPDATE);
        }
        appl_exit();
    }
    return 0;
}
