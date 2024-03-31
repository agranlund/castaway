#include "config.h"
#include <mint/mintbind.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>
#include <mint/falcon.h>
#include "gem.h"

extern void (*const jmp_table[8192*8])();
extern short vdo;
extern short snd;
extern short mch;
extern short hostcpu;
extern short vampire;

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


    uint32 cookie;
    cookie = 0; Getcookie('_CPU', &cookie); hostcpu = (int16) cookie;
    cookie = 0; Getcookie('_SND', &cookie); snd = (int16) cookie;
    cookie = 0; Getcookie('_VDO', &cookie); vdo = cookie >> 16;
    cookie = 0; Getcookie('_MCH', &cookie); mch = cookie >> 16;
    if (mch == 6) {
        hostcpu = 80;
        vdo = 3;
        #ifndef COLDFIRE
        vampire = 1;
        #endif
    }

    if (hostcpu < 20) { HALT("This program requires 68020+"); return -1; }
    #if defined(AC68080)
    if (hostcpu != 80) { HALT("This program requires 68080"); return -1; }
    #endif
    #if defined(VAMPIRE)
    if (!vampire) { HALT("This program requires Vampire"); return -1; }
    #endif

    StartEmulator(g_args, g_argv);
    HostExit();
    return 0;
}


#define C_UNION(x) { (uint32) x }
OBJECT rs_menu[] = {
	{ -1,  1,  4, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      0,0,    160,25 },
	{  4,  2,  2, G_BOX,    OF_NONE, OS_NORMAL,     C_UNION(0x1100L),                   0,0,    160,769 },
	{  1,  3,  3, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      2,0,    9,769 },
	{  2, -1, -1, G_TITLE,  OF_NONE, OS_NORMAL,     C_UNION(" Castaway"),               0,0,    9,769 },
	{  0,  5,  5, G_IBOX,   OF_NONE, OS_NORMAL,     C_UNION(0x0L),                      0,769,  160,19 },
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
	aes_global[0] = 0;
	int16 appId = appl_init();
	int16 aesVersion = aes_global[0];
    int16 vdiHandle = 0;
    int16 wndScreen = 0;
    int16 width, height;
	if (appId && (aesVersion > 0))
	{
        int16 dummy; int16 work_in[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2}; int16 work_out[57];
        vdiHandle = graf_handle(&dummy, &dummy, &dummy, &dummy);
        if (vdiHandle > 0) {
            v_opnvwk(work_in, &vdiHandle, work_out);
            width = work_out[0] + 1;
            height = work_out[1] + 1;
            wndScreen = wind_create(0, 0, 0, width, height);
            if (wndScreen) {
                wind_open(wndScreen, 0, 0, width, height);
            }
        }
        menu_bar(rs_menu, 1);
        graf_mouse(M_OFF, 0);
    }

    g_args = args;
    g_argv = argv;
    Supexec(supermain);
    
    if (appId) {
        if (aesVersion > 0) {
            menu_bar(rs_menu, 0);
            graf_mouse(M_ON, 0);
            graf_mouse(ARROW, 0);
        }
        appl_exit();
    }
    return 0;
}
