/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */
#ifndef _68000H
#define _68000H

#include "config.h"
#define CPU_TYPE        68000

/*
 * Exception vector numbers (trap numbers).
 */
#define RESET           0
#define BUSERR          2
#define ADDRESSERR      3
#define ILLINSTR        4
#define DIVZ            5
#define TRAPCHK         6
#define TRAPV           7
#define PRIV            8
#define TRACE           9
#define LINE_A          10
#define LINE_F          11
#if CPU_TYPE != 68000
#define FORMATERR       14
#endif
#define AUTOINT1        25
#define AUTOINT2        26
#define AUTOINT3        27
#define AUTOINT4        28
#define AUTOINT5        29
#define AUTOINT6        30
#define AUTOINT7        31
#define TRAP0           32 + 0
#define TRAP1           32 + 1
#define TRAP2           32 + 2
#define TRAP3           32 + 3
#define TRAP4           32 + 4
#define TRAP5           32 + 5
#define TRAP6           32 + 6
#define TRAP7           32 + 7
#define TRAP8           32 + 8
#define TRAP9           32 + 9
#define TRAP10          32 + 10
#define TRAP11          32 + 11
#define TRAP12          32 + 12
#define TRAP13          32 + 13
#define TRAP14          32 + 14
#define TRAP15          32 + 15

/* Group 0 exceptions (address error / bus error) */
extern void     ExceptionGroup0(
        int number,             /* trap number */
        unsigned long address,  /* fault address */
        int ReadWrite);         /* read = true, write = false */
/* Group 1 exceptions: illegal instruction, privilege violation, interrupts */
extern void     ExceptionGroup1(int number);
//extern void     ExceptionGroup1_TRAPV();
#define         ExceptionGroup1_TRAPV() ExceptionGroup1(TRAPV)
extern void     Interrupt(int number, int level);
/* Group 2 exceptions: traps, divide by zero */
extern void     ExceptionGroup2(int number);

extern void     HWReset(void);  /* Reset */
extern void     IllIns();   /* Illegal instruction */
extern void     Line_A();   /* Axxx instruction opcode  */
extern void     Line_F();   /* Fxxx instruction opcode */
extern void     Stop(void);     /* Stop instruction */

//extern unsigned char tracemode;	//For Trace Switching
//extern unsigned short SaveWordTrc;
//extern unsigned char GetTrc;

extern void (*const jmp_table[8192*8])();

register struct t_cpu* cpup __asm__("a6");
register unsigned long  pc __asm__("a5");
register uint8* membase __asm__("a4");
register unsigned short ccr __asm__("d7");

struct t_cpu
{
    int8*   membase;
    uint32  reg[16];
    uint32  usp;
    uint32  ssp;
    uint16  status;
    uint16  recalc_int;
    uint16  int0;
    int16   state;   /* 0, 1, 2 while processing exceptions, -1 executing, -2 stopped, -3 shutdown */
    uint32  dfc, sfc, vbr;           // 68010
    uint32  caar, cacr, isp, msp;    // 60820
    int32   type;    /* 68000, 68010 */
};
extern struct t_cpu cpu;

#define ins7 (inst & 7)
#define ins15 (inst & 15)

/*
 * Interrupts
 *
 * The state of interrupt lines IPL0-IPL2 is stored in the variable
 * intpri. The emulator acknowledges interrupts by calling QueryIRQ().
 * It expects the interrupt vector number to be returned.
 * WARNING:
 * Never update intpri asynchronously. Use the CPUEvent() function
 * to force CPURun() to return before updating intpri.
 */
//extern int      intpri;
extern int      QueryIRQ(int level); /* get interrupt vector number */

//#define GetS() (!!(status&0x2000))
//#define GetFC2() GetS()
#define GetFC2() (!!(cpup->status&0x2000))
#define GetFC1() ((address & MEMADDRMASK) == (pc & MEMADDRMASK))
#define GetFC0() ((address & MEMADDRMASK) != (pc & MEMADDRMASK))
#define GetFC() (((GetFC2() << 1) | GetFC1()) << 1) | GetFC0()


/*

// a5 is permanently reserved as emulated PC (in host address space)
// d6 is temporarily reserved as current instruction
// a4 is temporarily reserved as cycle counter
// usp is abused for stack restore
static inline short EmulateCPU() {

    #ifndef COLDFIRE
        // m68k
        #define CPU_START_BLOCK() \
            "move.l a6,-(sp)\n\t"\
            "move.l sp,a6\n\t"\
            "subq.l #4,a6\n\t"\
            "move.l a6,usp\n\t"\
            "move.l #_cpu,a6\n\t"\
            "move.l %0,d5\n\t"

        #define CPU_END_BLOCK()\
            "move.l (sp)+,a6\n\t"

        #define CPU_FETCH_AND_EXECUTE()\
            "move.w  (a5)+,d6\n\t"\
            "jsr     ([_jmp_table+(4096*8*4),d6.w*4])\n\t"

    #else
        // coldfire
        extern uint32 OperStackRestore;
        #define CPU_START_BLOCK()\
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
        : : "i"(CYCLES_PER_RUN)\
        : "d0","d1","d2","d3","d4","d5","d6","d7","a0","a1","a2","a3","a4","a5","a6","cc","memory" );
    #endif

    // return cycles
    register short ret;
    __asm__ volatile (\
        "move.w  %1,%0\n\t"\
        "sub.w   d5,%0\n\t"\
    : "=r"(ret) : "i"(CYCLES_PER_RUN): "cc");
    return ret;
}
*/



#ifdef DEBUG
extern int             stop_on;
extern int             verb_on;
/* go to sleep (gives debugger a chance to attach) */
void            DebugStop(void);
#ifdef TRACEBACK
/*
 * execution trace
 */
struct state {
    uint32          reg[16];
    uint32          stack[8];
    uint32          pc, usp, ssp;
    uint16          inst[5];
    uint16          sr;

};
extern int             hide_supervisor;
extern unsigned long   instcnt;
extern int             tbi;
extern struct state    traceback[TRACEBACK];
extern int             trace_on;
/* print execution trace of last TRACEBACK instructions and sleep */
void            TraceStop(void);
#endif
#endif
#endif

