#include "config.h"
#include "68000.h"
#include "mem.h"
#include "st.h"
#include "setjmp.h"

#define ENABLE_FRAMEPACING      0
#define ALLOW_FRAMESKIP         0

#define MHZ                     8
#define CYCLES_PER_RUN          ((400 * MHZ) / 8)
#define CYCLES_PER_LINE         ((512 * MHZ) / 8)
#define CYCLES_BORDER           ((96 * MHZ) / 8)

#define START_VISIBLE_SCREEN    64
#define END_VISIBLE_SCREEN      264
#define START_VBLANK            313

#if DEBUG_CPU
#define DBGPRINT(...)   DBG(__VA_ARGS__)
#else
#define DBGPRINT(...)
#endif

extern short fdc_transfert;
extern unsigned int fdc_start_adr,fdc_end_adr,fdc_current_adr;
extern unsigned char fdc_buffer[8000];
extern unsigned char fdc_motor;

char g_disk_name [ 252 ];
char g_disk_ext[ 4 ];
int  opt_framepacing;
int g_running;
jmp_buf g_term_jumpbuf;


// a5 is permanently reserved as emulated PC (in host address space)
// d6 is temporarily reserved as current instruction
// d5 is temporarily reserved as cycle counter
// usp is abused for stack restore
static inline short EmulateCPU() {

    #ifndef COLDFIRE
        // m68k
        #define CPU_START_BLOCK() \
            "move.l sp,a0\n\t"\
            "subq.l #4,a0\n\t"\
            "move.l a0,usp\n\t"\
            "move.l %0,d5\n\t"

        #define CPU_END_BLOCK()

        #define CPU_FETCH_AND_EXECUTE()\
            "move.w  (a5)+,d6\n\t"\
            "jsr     ([_jmp_table+(4096*8*4),d6.w*4])\n\t"

    #else
        // coldfire
        extern uint32 OperStackRestore;
        #define CPU_START_BLOCK()\
            "move.l sp,a0\n\t"\
            "subq.l #4,a0\n\t"\
            "move.l a0,_OperStackRestore\n\t"\
            "move.l %0,d5\n\t"\
            "clr.l  d6\n\t"

        #define CPU_END_BLOCK()

        #define CPU_FETCH_AND_EXECUTE()\
            "move.w  (a5)+,d6\n\t"\
            "move.l  #_jmp_table,a0\n\t"\
            "move.l  (a0,d6.l*4),a0\n\t"\
            "jsr     (a0)\n\t"

    #endif


    #if CHAINED_OPS
        __asm__ volatile (\
            CPU_START_BLOCK()\
            CPU_FETCH_AND_EXECUTE()\
            CPU_END_BLOCK()\
        : : "i"(CYCLES_PER_RUN)\
        : "d0","d1","d2","d3","d4","d5","d6","d7","a0","a1","a2","a3","a4","a5","a6","cc","memory" );
    #else
        __asm__ volatile (\
            CPU_START_BLOCK() \
            "loop%=:\n\t"\
            CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE()\
            CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE()\
            "tst.w   d5\n\t"\
            "bgt     loop%=\n\t"\
            CPU_END_BLOCK()\
        : : "i"(CYCLES_PER_RUN)\
        : "d0","d1","d2","d3","d4","d5","d6","d7","a0","a1","a2","a3","a5","cc","memory" );
    #endif

    // return cycles
    register short ret;
    __asm__ volatile (\
        "move.w  %1,%0\n\t"\
        "sub.w   d5,%0\n\t"\
    : "=r"(ret) : "i"(CYCLES_PER_RUN): "d5", "cc");
    return ret;
}


//extern short EmulateCPU();

void AtariST()
{
	short delay_fdc_motor=0;
	unsigned long oldpend,newpend;
	short counter_scanline=1;

	short vsyncpend=0;
	short hsyncpend=0;
	short hsync=0;

#if DEBUG_SPEED
    long totalseconds = 0;
    long averagecycles = 0;
    long totalcycles = 0;
    uint32 timenow = *((volatile uint32*)0x4ba);
    uint32 timeend = timenow + (1 * 200);
#endif

    g_running = 1;
	while (g_running)
    {
        // Emulate CPU
        short cycleco = EmulateCPU();
        //DBG("count = %d", cycleco);

        #if DEBUG_SPEED
        totalcycles += cycleco;
        if (*((volatile uint32*)0x4ba) >= timeend)
        {
            DBG("%ld cycles per second", totalcycles);
            averagecycles += totalcycles;
            totalcycles = 0;
            totalseconds++;
            if (totalseconds > 19) {
                DBG("%ld average", averagecycles / 20);
				exit(0);
            }
            timenow = *((volatile uint32*)0x4ba);
            timeend = timenow + (1 * 200);
        }
        #endif


		//MFP timer A delay mode
		if (mfp_ascale>1) {
			mfp_acount-=(mfp_ascale*cycleco);
			if (mfp_acount<=0) {
				do {mfp_acount+=mfp_tadr;} while (mfp_acount<=0);
				oldpend=mfp_ipra; newpend=(oldpend|0x20)&mfp_iera;
				if (newpend!=oldpend) {mfp_ipra=newpend; cpup->recalc_int=1;}
			}
		}
		//MFP timer B delay mode
		if (mfp_bscale>1) {
			mfp_bcount-=(mfp_bscale*cycleco);
			if (mfp_bcount<=0) {
				do {mfp_bcount+=mfp_tbdr;} while (mfp_bcount<=0);
				oldpend=mfp_ipra; newpend=(oldpend|0x1)&mfp_iera;
				if (newpend!=oldpend) {mfp_ipra=newpend; cpup->recalc_int=1;}
			}
		}
		//MFP timer C delay mode
		if (mfp_cscale>1) {
			mfp_ccount-=(mfp_cscale*cycleco);
			if (mfp_ccount<=0) {
				do {mfp_ccount+=mfp_tcdr;} while (mfp_ccount<=0);
				oldpend=mfp_iprb; newpend=(oldpend|0x20)&mfp_ierb;
				if (newpend!=oldpend) {mfp_iprb=newpend; cpup->recalc_int=1;}
			}
		}
		//MFP timer D delay mode
		if (mfp_dscale>1) {
			mfp_dcount-=(mfp_dscale*cycleco);
			if (mfp_dcount<=0) {
				do {mfp_dcount+=mfp_tddr;} while (mfp_dcount<=0);
				oldpend=mfp_iprb; newpend=(oldpend|0x10)&mfp_ierb;
				if (newpend!=oldpend) {mfp_iprb=newpend; cpup->recalc_int=1;}
			}
		}
		
		//Count vid_hbls
		hsync-=cycleco;

		//Update video address
		if (vid_hbl>=START_VISIBLE_SCREEN && vid_hbl<END_VISIBLE_SCREEN)
        {
			if (hsync>=96&&hsync<=96+320)
                vid_adr+=(cycleco>>1)&(~1);

            if ((counter_scanline==1)&&(hsync<=CYCLES_BORDER))
            {
				counter_scanline=0;
                //Timer-B event count mode
                if (mfp_bscale==1)
                {
                    mfp_bcount-=1048576;
                    if (mfp_bcount<=0)
                    {
                        mfp_bcount=mfp_tbdr;
                        oldpend=mfp_ipra;
                        newpend=(oldpend|0x1)&mfp_iera;
                        if (newpend!=oldpend)
                        {
                            mfp_ipra=newpend;
                            cpup->recalc_int=1;
                        }
                    }
                }
			}
		}

		if (hsync<=0)
        {
			counter_scanline=1;
			vid_hbl++;
            hsync+=CYCLES_PER_LINE;
			
			if (hsyncpend==0)
            {
				hsyncpend=1;
                cpup->recalc_int=1;
			}

			if (fdc_transfert)
            {
                for (short i=0;i<16;i++) {
                    *(unsigned char*)(membase+fdc_start_adr+fdc_current_adr+i)=*(unsigned char*)(fdc_buffer+fdc_current_adr+i);
                }

                fdc_current_adr+=16;
                dma_adrl = (fdc_start_adr+fdc_current_adr) & 0xff; 
                dma_adrm = ((fdc_start_adr+fdc_current_adr) >> 8) & 0xff; 
                dma_adrh = ((fdc_start_adr+fdc_current_adr) >> 16) & 0xff; 
                if (fdc_start_adr+fdc_current_adr>=fdc_end_adr)
                {
                    mfp_iprb |= (0x80 & mfp_ierb);
                    mfp_gpip &= ~0x20;
                    delay_fdc_motor=0;
                    fdc_transfert=0;
                    dma_sr = 1;
                    dma_scr = 0;
                }
			}

			
			//Trigger event counters for visible scan-lines
			if (vid_hbl>=START_VISIBLE_SCREEN && vid_hbl<END_VISIBLE_SCREEN)
            {
 				//Do IO every 64 vid_hbls
				if (!(vid_hbl&63))
                {
                    // Handle event outside the emulation
                    HostEvents();
                    if (!g_running) {
                        break;
                    }

                    // Update ikbd
                    {
                        int maxx = (vid_shiftmode == 0) ? 319 : 639;
                        int maxy = (vid_shiftmode != 2) ? 199 : 399;
                        if (mouse.x < 0) mouse.x = 0;
                        if (mouse.y < 0) mouse.y = 0;
                        if (mouse.x > maxx) mouse.x = maxx;
                        if (mouse.y > maxy) mouse.y = maxy;
                        IkbdLoop();
                    }
					
					//Generate FDC interrupt in mfp?
					if (!(mfp_gpip & 0x20))
                    {
						mfp_iprb |= 0x80;
						mfp_iprb &= mfp_ierb;
						cpup->recalc_int=1;
					} 
					
					//Generate ACIA interrupt in mfp?
					IkbdWriteBuffer();
					
					if (!(mfp_gpip & 0x10))
                    {
						mfp_iprb |= 0x40;
						mfp_iprb &= mfp_ierb;
						cpup->recalc_int=1;
					}
				}

                // next scanline
                vid_adr = (vid_baseh<<16)+(vid_basem<<8)+(vid_hbl-63)*160;

				/*Timer-A event count mode
				if (mfp_ascale==1) {
					mfp_acount-=1048576;
					if (mfp_acount<=0) {
						mfp_acount=mfp_tadr;
						oldpend=mfp_ipra; newpend=(oldpend|0x20)&mfp_iera;
						if (newpend!=oldpend) {mfp_ipra=newpend; cpup->recalc_int=1;}
					}
				}*/
			}

			//Vertical blank?
			else if (vid_hbl>=START_VBLANK)
            {
                HostVblank();

				vid_adr=(vid_baseh<<16)+(vid_basem<<8);
                vid_hbl=0;
				vsyncpend=1;
                cpup->recalc_int=1;
				
                //Do fdc spinup
				if (fdc_motor)
                {
					if (delay_fdc_motor>75)
                    {
						fdc_status &= ~0x80;
						delay_fdc_motor=0;
						fdc_motor=0;
					}
					else delay_fdc_motor++;
				}
				if (disk[0].changed)
                {
                    disk[0].changed++;
                    fdc_status |= 0x40;
                    if (disk[0].changed>=75)
                    {
                        disk[0].changed=0;
                        fdc_status &= ~0x40;
                    }
				}

				//Output some sound
                #if 1
                sampos = 0;
                #else
                #ifdef PSGSOUND
				if (checkedsound)
                    fillsoundbuffer();
                #endif
                #endif
			}
		}
		
		//Recalculate interrupts?
		if (cpup->recalc_int==1)
        {
			short mfp_int = 0;
            if ((cpup->status & 0x700) <= 0x600)
            {
				//Mfp interrupt
				{
					int n, number;
					uint16 imr, ipr, isr, irr;
					int in_request;
					//Find in_request and in_service
					imr = (mfp_imra<<8)+mfp_imrb;
					ipr = (mfp_ipra<<8)+mfp_iprb;
					irr = imr & ipr;
					isr = (mfp_isra<<8) + mfp_isrb;
					//in_request higher than in_service?
					if (irr>isr)
                    {
						//Find highest set bit
						for (in_request = 15; in_request > 0; in_request--)
                        {
							if (irr & 0x8000) break;
							irr <<= 1;
						}
						isr = 1 << in_request;
						//Set interrupt in service bits in MFP
						if (mfp_ivr & 0x8)
                        {
							mfp_isra |= isr >> 8;
							mfp_isrb |= isr;
						}
                        else
                        {
							mfp_isra &= (~isr) >> 8;
							mfp_isrb &= ~isr;
						}
						//Clear interrupt pending bits in MFP
						mfp_ipra &= ~(isr >> 8);
						mfp_iprb &= ~isr;
						//Pass interrupt to cpu
						number = in_request | (mfp_ivr & 0xf0);
						Interrupt(number, 6);
						mfp_int=1;
					}
				}
			}
			if (!mfp_int)
            {
				if (vsyncpend==1 && ((cpup->status & 0x0700) <= 0x400))
                {
					Interrupt(AUTOINT4, 4);
					vsyncpend=0;
				}
                else if (hsyncpend==1 && ((cpup->status & 0x700) <= 0x200))
                {
					Interrupt(AUTOINT2, 2);
					hsyncpend=0;
				}
			}
			
			if (!vsyncpend&&!hsyncpend)
                cpup->recalc_int=0;
		}
    }
    DBG("Exit emulation");
    longjmp(g_term_jumpbuf, 1);
}


struct t_heap {
    uint16 type;
    uint32 addr;
    uint32 size;
    uint32 free;
    uint32 ptr;
};
#define maxheaps 16
struct t_heap heap[maxheaps];

uint32 heapAlloc(short i, uint32 size, uint32 align, short type)
{
    uint32 mask = align - 1;
    uint32 asiz = size + mask;
    if ((heap[i].type == type) && (heap[i].free >= asiz)) {
        uint32 start = (heap[i].ptr + mask) & ~mask;
        uint32 end = start + size;
        uint32 before = start - heap[i].ptr;
        uint32 after = (heap[i].ptr + heap[i].free) - end;
        if (after > before) {
            heap[i].ptr = end;
            heap[i].free = after;
        } else {
            heap[i].free = before;
        }
        return start;
    }
    return 0;
}

uint32 AllocateMem(uint32 size, uint32 align, uint16 type) {
    uint32 ptr = 0;
    for (short i=0; (i<maxheaps) && (ptr == 0); i++) {
        ptr = heapAlloc(i, size, align, type);
    }

    if (ptr == 0) {
        for (short i=0; i<maxheaps; i++)
        {
            if (heap[i].size == 0) {
                uint32 asiz = size + align - 1;
                uint32 buf = HostAlloc(asiz, type);
                if (buf == 0) {
                    return 0;
                }
                heap[i].type = type;
                heap[i].addr = buf;
                heap[i].size = asiz;
                heap[i].free = asiz;
                heap[i].ptr = buf;
                ptr = heapAlloc(i, size, align, type);
                break;
            }
        }
    }
    if (ptr) {
        memset((void*)ptr, 0, size);
    }
    return ptr;
}

int InsertDisk(int num) {
    static int currentDiskNumer = 0;
    if (num == currentDiskNumer) {
        return 1;
    }

    FDCeject(0);
    sprintf(disk[0].name, "%s.%s", g_disk_name, g_disk_ext);
    if (num > 1) {
        int extlen = strlen(g_disk_ext);
        int len = strlen(disk[0].name);
        int offs = strlen(g_disk_ext) >= 3 ? -1 : 0;
        disk[0].name[len+offs+0] = '0' + num;
        disk[0].name[len+offs+1] = 0;
        if (FDCInit(0) == 0) {
            currentDiskNumer = num;
            return 1;
        }
        sprintf(disk[0].name, "%s.%s", g_disk_name, g_disk_ext);
    }
    if (FDCInit(0) == 0) {
        currentDiskNumer = num;
        return 1;
    }
    return 0;
}

void QuitEmulator()
{
    DBG("QuitEmu");
    g_running = 0;
}

int StartEmulator(int args, char* argv[])
{
    DBG("Init");
    char* disk = (args > 1) ? argv[1] : DISKA;
    strcpy(g_disk_name, disk);
    char* ext = strrchr(g_disk_name, '.');
    if (!ext) {
        HALT("invalid disk name");
        return -1;
    }
    *ext = 0;
    strncpy(g_disk_ext, ext+1, 3);
    display_mode = COL4;
    vid_shiftmode = display_mode;
    Init();

    DBG("Floppy %s.%s", g_disk_name, g_disk_ext);
    InsertDisk(1);

    DBG("Host setup");
    if (!HostInit()) {
        return 0;
    }

    DBG("Run");
    opt_framepacing = FRAMEPACING_DEFAULT;


    if (setjmp(g_term_jumpbuf) == 0)
    {
        AtariST();
    }



    DBG("Run done");
    return 0;
}
