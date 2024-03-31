/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* 68000.c - 68000 emulator jump table and misc subroutines
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  09.06.2002  JH  Use of mmap for memory access discontinued
*  12.06.2002  JH  Correct bus error/address error exception stack frame
*  13.06.2002  JH  Merged in Martin's BCD implementation (untested),
*                  completed jump table. STOP is now the only
*                  unimplemented instruction.
*  14.06.2002  JH  Implemented STOP, shutdown CPU after multiple bus errors.
*                  Removed inst parameter from CPU opcode functions.
*  19.06.2002  JH  CPURun() returns # of executed instructions.
*  20.06.2002  JH  added yet another SR implementation variant.
*  02.07.2002  JH  Support different CPU types. Removed MOVE CCR,<EA> from
*                  68000 jump table.
*  20.08.2002  JH  Fixed CPU shutdown.
*  27.08.2002  JH  Bugfix: S and T flag incorrectly reported for group 0 exceptions
*                  Implemented additional 68010 registers and instructions.
*  31.08.2002  JH  Implemented M68010 exception stack frames.
*  08.10.2002  JH  Implemented Trace exception
*/
static char     sccsid[] = "$Id: 68000.c,v 1.18 2002/10/10 19:52:11 jhoenig Exp $";
#include "config.h"
#include <setjmp.h>
#include "68000.h"
#include "op68k.h"
#include "mem.h"
#include "st.h"
#include "proto.h"
/*
#if (CPU_TYPE == 68000 || CPU_TYPE == 68008)
#include "op68000.c"
#elif (CPU_TYPE == 68010) // broken
#include "op68010.c"
#endif
*/

struct t_cpu cpu;

#ifdef COLDFIRE
uint32 OperStackRestore;
#endif

#if CPU_TYPE == 68010
#define cpu_vbr cpup->vbr
#else
#define cpu_vbr 0
#endif



#if 0
// a5 is permanently reserved as emulated PC (in host address space)
// d6 is temporarily reserved as current instruction
// a4 is temporarily reserved as cycle counter
// usp is abused for stack restore
short EmulateCPU() {
    register short ret __asm__("d0");

    #ifndef COLDFIRE
        // m68k

        #define CPU_START_BLOCK() \
            "move.l sp,a6\n\t"\
            "subq.l #4,a6\n\t"\
            "move.l a6,usp\n\t"\
            "move.l #_cpu,a6\n\t"\
            "move.w %1,d5\n\t"

        #define CPU_END_BLOCK()\
            "move.w  %1,%0\n\t"\
            "sub.w   d5,%0\n\t"\

        #define CPU_FETCH_AND_EXECUTE()\
            "move.w  (a5)+,d6\n\t"\
            "jsr     ([_jmp_table+(4096*8*4),d6.w*4])\n\t"

    #else
        // coldfire
        extern uint32 OperStackRestore;
        #define CPU_START_BLOCK()\
            "movem.l d2-d7/a2-a6,-(sp)\n\t"\
            "move.l a6,-(sp)\n\t"\
            "move.l sp,a6\n\t"\
            "subq.l #4,a6\n\t"\
            "move.l a6,_OperStackRestore\n\t"\
            "move.l #_jmp_table,a6\n\t"\
            "move.l %0,d5\n\t"\
            "clr.l  d6\n\t"

        #define CPU_END_BLOCK()\
            "move.l (sp)+,a6\n\t"

        #define CPU_FETCH_AND_EXECUTE()\
            "move.w  (a5)+,d6\n\t"\
            "move.l  (a6,d6.l*4),a0\n\t"\
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
            CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE()\
            CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE() CPU_FETCH_AND_EXECUTE()\
            "tst.w   d5\n\t"\
            "bgt     loop%=\n\t"\
            CPU_END_BLOCK()\
        : "=r"(ret) : "i"(CYCLES_PER_RUN)\
        : "d1","d2","d3","d4","d5","d6","d7","a0","a1","a2","a3","a4","a5","cc","memory" );
    #endif

    return ret;
}
#endif



//extern int disass;
//extern void StartDisass();

void ExceptionGroup0(int number, unsigned long address, int ReadWrite)
{
    //DBG("ExceptionGroup0: %d", number);
    short sr = GetSRW(),
    context = 0;
#ifdef DEBUG
    ON_TRAP(number);
    assert(cpup->state != -3);
#endif
//	if ((exaddress!=address)&&(exaddress+1!=address)&&(exaddress+2!=address)&&(exaddress+3!=address)){
		if (cpup->state == 0) {
			cpup->state = -3;
			//longjmp(trap_buf, 1);
		}
#if CPU_TYPE == 68000
		else if (cpup->state > 0) {
			context |= 0x8;
		}
		if (ReadWrite) context |= 0x10;
#else
		if (ReadWrite) context |= 0x100;
#endif
		if (GetS()) context |= 0x4;
		if (ReadWrite && ((address & MEMADDRMASK) == (pc & MEMADDRMASK))) context |= 0x2;
		else context |= 0x1;
		cpup->state = 0; /* begin group 0 exception processing */
		SetS(1); SetT(0);
#if CPU_TYPE == 68010
		cpup->reg[15] -= 44; /* Rerun info */
		cpup->reg[15] -= 4; SetMemL(cpup->reg[15], address); /* fault address */
		cpup->reg[15] -= 2; SetMemW(cpup->reg[15], context);
		cpup->reg[15] -= 2; SetMemW(cpup->reg[15], 0x8000 | (number * 4));
		cpup->reg[15] -= 4; SetMemL(cpup->reg[15], GetPC());
		cpup->reg[15] -= 2; SetMemW(cpup->reg[15], sr);
#else
		cpup->reg[15] = cpup->reg[15] - 14;
        uint32 addr = (uint32)membase + (cpup->reg[15] & MEMADDRMASK);
		WriteW(addr + 0, context);
		WriteL(addr + 2, address);
		WriteW(addr + 6, inst);
		WriteW(addr + 8, sr);
		WriteL(addr + 10, GetPC());
#endif
		SetPC(ReadL(membase + cpu_vbr + (long)(number * 4)));

		/* end exception processing */
		cpup->state = -1;
//		exaddress=address;
		
//	}
	cpup->int0=1;
}


void ExceptionGroup1(int number)
{
    //DBG("ExceptionGroup1: %d", number);
    uint32 newpc = (number != TRAPV) ? GetPC() - 2 : GetPC();
    short sr = GetSRW();
    SetS(1); SetT(0);
    cpup->reg[15] -= 6;
    uint32 addr = (uint32)membase + (cpup->reg[15] & MEMADDRMASK);
    WriteW(addr + 0, sr);
    WriteL(addr + 2, newpc);
    SetPC(ReadL((uint32)membase + cpu_vbr + (number*4)));
	cpup->int0=1;
}

void Interrupt(int number, int level)
{
    // this function is called from outside emulation block.
    // must use cpup-> instead of cpup->
	uint32 sp;
    short sr = GetSRW();
    SetI(level); SetS(1); SetT(0);
    cpup->reg[15] -= 6;
    uint32 addr = (uint32)membase + (cpup->reg[15] & MEMADDRMASK);
    WriteW(addr + 0, sr);
    WriteL(addr + 2, GetPC());
    SetPC(ReadL((uint32)membase + cpu_vbr + (number*4)));
}

void ExceptionGroup2(int number)
{
    //DBG("ExceptionGroup2: %d", number);
    short sr = GetSRW();
    SetS(1); SetT(0);
    cpup->reg[15] -= 6;
    uint32 addr = (uint32)membase + (cpup->reg[15] & MEMADDRMASK);
    WriteW(addr + 0, sr);
    WriteL(addr + 2, GetPC());
    SetPC(ReadL((uint32)membase + cpu_vbr + (number*4)));
}

unsigned long Trace()
{
    /*
	uint32 sp;
	short sr;
	register unsigned long cycleco=0;
	register unsigned long address,timeinst;
	register int8 *mymembase=membase;
	register int8 *myrombase=rombase;
	register uint32 myinst;
	
	address = pc&MEMADDRMASK;
	//	disass=1;
	cpup->int0=0;
	if (address<MEMSIZE) myinst=biginst=ReadSL(mymembase+address);
	else myinst=biginst=ReadSL(myrombase+address);
#ifdef DISASS
	if (disass==1) StoreTrace();
#endif
	pc+=2;
	//cycleco+=(*jmp_table[(myinst<<16)>>19])(reg);
	cycleco+=(*jmp_table[(myinst<<16)>>19])();
	
	if (!cpup->int0){
		sr = GetSRW();
		SetS(1); SetT(0);
		sp=reg[15];
		sp-=4; SetMemL(sp, GetPC());
		sp-=2; SetMemW(sp, sr);
		reg[15]=sp;
		SetPC (GetMemL(TRACE*4+vbr));
		return cycleco+34;
	}
	*/
	return 0;
}

OperBegin(IllIns)
    ExceptionGroup1(ILLINSTR);
    OperExit(4);    // cycles?

OperBegin(Line_A)
    ExceptionGroup1(LINE_A);
    OperExit(4);    // cycles?

OperBegin(Line_F)
    ExceptionGroup1(LINE_F);
    OperExit(4);    // cycles?

OperBegin(Stop)
    if (!GetS())
        ExceptionGroup1(PRIV);
    SetSRW(GetMPCW());
	pc+=2;
    /*pc -= 2;
    cpup->state = -2; // stopped 
	vsyncpend=1;
	cpup->recalc_int=1;*/
    //longjmp(trap_buf, 1);
    OperExit(4);    // cycles?

void HWReset(void)
{
    cpup = &cpu;
    // this can be called outside of emulation block
    cpup->reg[15] = ReadL(cpup->membase);
    SetPC(ReadL(cpup->membase + 4));
    SetSRW(0x2700);
    cpup->state = -1; /* running */
    cpup->type = CPU_TYPE;

    memset(vid_colr, 0, 200*16*2);
    memset(vid_colf, 0, 200*2);
    //__asm__ ("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
}
