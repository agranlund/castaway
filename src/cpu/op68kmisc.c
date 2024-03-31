/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - 68k miscellaneous instructions
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  12.06.2002  JH  Correct bus error/address error exception stack frame
*  14.06.2002  JH  Implemented STOP, shutdown CPU after multiple bus errors.
*                  Removed inst parameter from CPU opcode functions.
*  09.07.2002  JH  STOP should work now as expected.
*  10.07.2002  JH  Fixed X, C and Z-Flag for NEGX, X and C for NEG.
*  27.08.2002  JH  Some clarifications in op68k.h
*                  Implemented additional 68010 registers and instructions.
*  31.08.2002  JH  Implemented M68010 exception stack frames.
*  10.09.2002  JH  Fixed MOVE to/from CCR: word size
*  15.09.2002  JH  Minor SR/Bcc/Scc optimization.
*  25.10.2002  JH  CHKTRACE no longer optional, improved implementation.
*                  PC correct now if MOVE to SR raises privilege violation.
*/
#ifndef PROTOH
static char     sccsid[] = "$Id: op68kmisc.c,v 1.11 2002/10/25 21:56:27 jhoenig Exp $";
#define OPHANDLER
#include "68000.h"
#include "op68k.h"

extern uint32 psg_reg_vol,psg_cur_reg_vol;


static inline void PushW(uint16 x) {
    cpup->reg[15] -= 2;
    WriteW(membase + (cpup->reg[15] & MEMADDRMASK), x);
}

static inline void PushL(uint32 x) {
    cpup->reg[15] -= 4;
    WriteL(membase + (cpup->reg[15] & MEMADDRMASK), x);
}

static inline uint16 PopW() {
    register uint16 result = ReadW(membase + (cpup->reg[15] & MEMADDRMASK));
    cpup->reg[15] += 2;
    return result;
}

static inline uint32 PopL() {
    register uint32 result = ReadL(membase + (cpup->reg[15] & MEMADDRMASK));
    cpup->reg[15] += 4;
    return result;
}



/*
* Opfuncs.
*/
#define OMoves(Code, Decl, CalcEA, GetReg, SetReg, GetMem, SetMem, retval) \
OperBegin(Code) \
    Decl(source)\
    unsigned long address;\
    unsigned short ext = GetMPCW();\
    unsigned short index = ext >> 12;\
    if (!GetS()) { ExceptionGroup1(PRIV); };\
    pc += 2;\
    CalcEA(address, ins7)\
    if (ext & 0x800) { \
	GetReg(index, source)\
	SetMem(address, source)\
    } else {\
	GetMem(address, source)\
	SetReg(index, source)\
    }\
    OperExit(retval);

#define OMoveToSR(Code, Get1, retval)\
OperBegin(Code)\
    DW(source)\
    if (!GetS()) ExceptionGroup1(PRIV);\
    Get1(source, ins7)\
    SetSRW(source);\
    OperExit(retval);

#if 1
#define DoBtst32(target,source) { source &= 31; __asm__ volatile ( INSTR_BEGIN "btst.l %1,%0\n\t" INSTR_END : : "d"(target), "d"(source) : "cc" ); };
#define DoBtst8(target,source)  { source &=  7; __asm__ volatile ( INSTR_BEGIN "btst.b %1,%0\n\t" INSTR_END : : "d"(target), "d"(source) : "cc" ); };
#define DoBchg32(target,source) { source &= 31; __asm__ volatile ( INSTR_BEGIN "bchg.l %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };
#define DoBchg8(target,source)  { source &=  7; __asm__ volatile ( INSTR_BEGIN "bchg.b %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };
#define DoBclr32(target,source) { source &= 31; __asm__ volatile ( INSTR_BEGIN "bclr.l %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };
#define DoBclr8(target,source)  { source &=  7; __asm__ volatile ( INSTR_BEGIN "bclr.b %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };
#define DoBset32(target,source) { source &= 31; __asm__ volatile ( INSTR_BEGIN "bset.l %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };
#define DoBset8(target,source)  { source &=  7; __asm__ volatile ( INSTR_BEGIN "bset.b %2,%0\n\t" INSTR_END : "=d"(target): "0"(target), "d"(source) : "cc" ); };

#define DoNegL(target,source)   { __asm__ volatile ( INSTR_BEGIN "neg.l %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNegxL(target,source)  { __asm__ volatile ( INSTR_BEGIN "negx.l %0\n\t" INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNotL(target,source)   { __asm__ volatile ( INSTR_BEGIN "not.l %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };

#ifndef COLDFIRE
#define DoNegW(target,source)   { __asm__ volatile ( INSTR_BEGIN "neg.w %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNegxW(target,source)  { __asm__ volatile ( INSTR_BEGIN "negx.w %0\n\t" INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNotW(target,source)   { __asm__ volatile ( INSTR_BEGIN "not.w %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNegB(target,source)   { __asm__ volatile ( INSTR_BEGIN "neg.b %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNegxB(target,source)  { __asm__ volatile ( INSTR_BEGIN "negx.b %0\n\t" INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#define DoNotB(target,source)   { __asm__ volatile ( INSTR_BEGIN "not.b %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" ); };
#endif

#define DoTstL(target,source)   { __asm__ volatile ( INSTR_BEGIN "cmp.l #0,%0\n\t"  INSTR_END : : "d"(target) : "cc" ); };
#define DoTstW(target,source)   { __asm__ volatile ( INSTR_BEGIN "cmp.w #0,%0\n\t"  INSTR_END : : "d"(target) : "cc" ); };
#define DoTstB(target,source)   { __asm__ volatile ( INSTR_BEGIN "cmp.b #0,%0\n\t"  INSTR_END : : "d"(target) : "cc" ); };

#define DoSt(target,source)     { __asm__ volatile ( INSTR_BEGIN "st  %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSf(target,source)     { __asm__ volatile ( INSTR_BEGIN "sf  %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoShi(target,source)    { __asm__ volatile ( INSTR_BEGIN "shi %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSls(target,source)    { __asm__ volatile ( INSTR_BEGIN "sls %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoScc(target,source)    { __asm__ volatile ( INSTR_BEGIN "scc %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoScs(target,source)    { __asm__ volatile ( INSTR_BEGIN "scs %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSne(target,source)    { __asm__ volatile ( INSTR_BEGIN "sne %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSeq(target,source)    { __asm__ volatile ( INSTR_BEGIN "seq %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSvc(target,source)    { __asm__ volatile ( INSTR_BEGIN "svc %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSvs(target,source)    { __asm__ volatile ( INSTR_BEGIN "svs %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSpl(target,source)    { __asm__ volatile ( INSTR_BEGIN "spl %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSmi(target,source)    { __asm__ volatile ( INSTR_BEGIN "smi %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSge(target,source)    { __asm__ volatile ( INSTR_BEGIN "sge %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSlt(target,source)    { __asm__ volatile ( INSTR_BEGIN "slt %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSgt(target,source)    { __asm__ volatile ( INSTR_BEGIN "sgt %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#define DoSle(target,source)    { __asm__ volatile ( INSTR_BEGIN "sle %0\n\t"  INSTR_END_NCC : "=d"(target): : "cc" ); };
#endif

/* SWAP */
OperBeginS(Op4840)
    #if 1
        uint32 target = GetRegL(ins7);
        __asm__ volatile ( INSTR_BEGIN "swap %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" );
        SetRegL(ins7, target);
    #else
        long source = GetRegL (ins7);
        long target = ((unsigned) source) >> 16;
        source <<= 16;
        target |= source;
        SetRegL (ins7, target);
        ClrCVSetNZ (target);
    #endif
    OperExitS(4);


/* EXT.W */
OperBeginS(Op4880)
    #if 1
        uint32 target = GetRegW(ins7);
        __asm__ volatile ( INSTR_BEGIN "ext.w %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" );
        SetRegW(ins7, target);
    #else
        char source = GetRegB (ins7);
        short target = source;
        SetRegW(ins7, target);
        ClrCVSetNZ (target);
    #endif
    OperExitS(4);

/* EXT.L */
OperBeginS(Op48c0)
    #if 1
        uint32 target = GetRegL(ins7);
        __asm__ volatile ( INSTR_BEGIN "ext.l %0\n\t"  INSTR_END : "=d"(target): "0"(target) : "cc" );
        SetRegL(ins7, target);
    #else    
        short source = GetRegW (ins7);
        long target = source;
        SetRegL (ins7, target);
        ClrCVSetNZ (target);
    #endif
    OperExitS(4);


#ifndef DoBtst8
#define DoBtst8(target,source) SetZI ((target>>(source & 7))&1);
#endif
#ifndef DoBtst32
#define DoBtst32(target,source) SetZI ((target>>(source & 31))&1);
#endif
#ifndef DoBchg8
#define DoBchg8(target,source) {uint32 mask=(1 << (source & 7)); SetZI(target&mask); target^=mask;}
#endif
#ifndef DoBchg32
#define DoBchg32(target,source) {uint32 mask=(1 << (source & 31)); SetZI(target&mask); target^=mask;}
#endif
#ifndef DoBclr8
#define DoBclr8(target,source) {uint32 mask=(1 << (source & 7)); SetZI(target&mask); target&=~mask;}
#endif
#ifndef DoBclr32
#define DoBclr32(target,source) {uint32 mask=(1 << (source & 31)); SetZI(target&mask); target&=~mask;}
#endif
#ifndef DoBset8
#define DoBset8(target,source) {uint32 mask=(1 << (source & 7)); SetZI(target&mask); target|=mask;}
#endif
#ifndef DoBset32
#define DoBset32(target,source) {uint32 mask=(1 << (source & 31)); SetZI(target&mask); target|=mask;}
#endif
#ifndef DoNegB
#define DoNegB(target,source) { SetV (target < 0 && -target < 0); target = -target; SetXC (target != 0); SetNZ(target); }
#endif
#ifndef DoNegW
#define DoNegW(target,source) { SetV (target < 0 && -target < 0); target = -target; SetXC (target != 0); SetNZ(target); }
#endif
#ifndef DoNegL
#define DoNegL(target,source) { SetV (target < 0 && -target < 0); target = -target; SetXC (target != 0); SetNZ(target); }
#endif
#define DoNegx(target,source) {\
    SetV (target < 0);\
    if (GetX()) {\
	SetXC(1);\
	target = -target;\
	target -= 1;\
    } else {\
	SetXC (target != 0);\
	target = -target;\
    }\
    SetV (GetV() && (target < 0));\
    ZeroZ(target!=0);\
    SetN(target < 0);\
}
#ifndef DoNegxB
#define DoNegxB DoNegx
#endif
#ifndef DoNegxW
#define DoNegxW DoNegx
#endif
#ifndef DoNegxL
#define DoNegxL DoNegx
#endif
#ifndef DoNotB
#define DoNotB(target,source) { target = ~target; ClrCVSetNZ(target); }
#endif
#ifndef DoNotW
#define DoNotW(target,source) { target = ~target; ClrCVSetNZ(target); }
#endif
#ifndef DoNotL
#define DoNotL(target,source) { target = ~target; ClrCVSetNZ(target); }
#endif
#ifndef DoTstB
#define DoTstB(target,source) ClrCVSetNZ(target);
#endif
#ifndef DoTstW
#define DoTstW(target,source) ClrCVSetNZ(target);
#endif
#ifndef DoTstL
#define DoTstL(target,source) ClrCVSetNZ(target);
#endif
#ifndef DoSt
#define DoSt(target,source) target = CCt ? 0xff : 0x00;
#endif
#ifndef DoSf
#define DoSf(target,source) target = CCf ? 0xff : 0x00;
#endif
#ifndef DoShi
#define DoShi(target,source) target = CChi ? 0xff : 0x00;
#endif
#ifndef DoSls
#define DoSls(target,source) target = CCls ? 0xff : 0x00;
#endif
#ifndef DoScc
#define DoScc(target,source) target = CCcc ? 0xff : 0x00;
#endif
#ifndef DoScs
#define DoScs(target,source) target = CCcs ? 0xff : 0x00;
#endif
#ifndef DoSne
#define DoSne(target,source) target = CCne ? 0xff : 0x00;
#endif
#ifndef DoSeq
#define DoSeq(target,source) target = CCeq ? 0xff : 0x00;
#endif
#ifndef DoSvc
#define DoSvc(target,source) target = CCvc ? 0xff : 0x00;
#endif
#ifndef DoSvs
#define DoSvs(target,source) target = CCvs ? 0xff : 0x00;
#endif
#ifndef DoSpl
#define DoSpl(target,source) target = CCpl ? 0xff : 0x00;
#endif
#ifndef DoSmi
#define DoSmi(target,source) target = CCmi ? 0xff : 0x00;
#endif
#ifndef DoSge
#define DoSge(target,source) target = CCge ? 0xff : 0x00;
#endif
#ifndef DoSlt
#define DoSlt(target,source) target = CClt ? 0xff : 0x00;
#endif
#ifndef DoSgt
#define DoSgt(target,source) target = CCgt ? 0xff : 0x00;
#endif
#ifndef DoSle
#define DoSle(target,source) target = CCle ? 0xff : 0x00;
#endif


#define DoChk(target,source) { SetN (source < 0); if (source < 0 || source > target) ExceptionGroup2(TRAPCHK); }
#define DoClr(target,source) { target = 0; ccr = Zflag | (ccr & ~(Cflag|Vflag|Nflag)); }
#define DoTas(target,source) { ClrCVSetNZ(target); target |= 0x80; }

#define DoBra(target,source) if (!source) source = FastMPCW (); pc += source;
#define DoBras(target,source) pc += source;
#define DoBsr(target,source) if (!source) { source = FastMPCW (); PushL(GetPC() + 2);} else {PushL(GetPC());} pc += source;
#define DoBsrs(target,source) PushL(GetPC()); pc += source;
#define DoBhi(target,source) if (!source) {source = FastMPCW (); if (CChi) {pc += source;} else { pc += 2;}} else {if (CChi) pc += source;};
#define DoBhis(target,source) if (CChi) pc += source;
#define DoBls(target,source) if (!source) {source = FastMPCW (); if (CCls) {pc += source;} else { pc += 2;}} else {if (CCls) pc += source;};
#define DoBlss(target,source) if (CCls) pc += source;
#define DoBcc(target,source) if (!source) {source = FastMPCW (); if (CCcc) {pc += source;} else { pc += 2;}} else {if (CCcc) pc += source;};
#define DoBccs(target,source) if (CCcc) pc += source;
#define DoBcs(target,source) if (!source) {source = FastMPCW (); if (CCcs) {pc += source;} else { pc += 2;}} else {if (CCcs) pc += source;};
#define DoBcss(target,source) if (CCcs) pc += source;
#define DoBne(target,source) if (!source) {source = FastMPCW (); if (CCne) {pc += source;} else { pc += 2;}} else {if (CCne) pc += source;};
#define DoBnes(target,source) if (CCne) pc += source;
#define DoBeq(target,source) if (!source) {source = FastMPCW (); if (CCeq) {pc += source;} else { pc += 2;}} else {if (CCeq) pc += source;};
#define DoBeqs(target,source) if (CCeq) pc += source;
#define DoBvc(target,source) if (!source) {source = FastMPCW (); if (CCvc) {pc += source;} else { pc += 2;}} else {if (CCvc) pc += source;};
#define DoBvcs(target,source) if (CCvc) pc += source;
#define DoBvs(target,source) if (!source) {source = FastMPCW (); if (CCvs) {pc += source;} else { pc += 2;}} else {if (CCvs) pc += source;};
#define DoBvss(target,source) if (CCvs) pc += source;
#define DoBpl(target,source) if (!source) {source = FastMPCW (); if (CCpl) {pc += source;} else { pc += 2;}} else {if (CCpl) pc += source;};
#define DoBpls(target,source) if (CCpl) pc += source;
#define DoBmi(target,source) if (!source) {source = FastMPCW (); if (CCmi) {pc += source;} else { pc += 2;}} else {if (CCmi) pc += source;};
#define DoBmis(target,source) if (CCmi) pc += source;
#define DoBge(target,source) if (!source) {source = FastMPCW (); if (CCge) {pc += source;} else { pc += 2;}} else {if (CCge) pc += source;};
#define DoBges(target,source) if (CCge) pc += source;
#define DoBlt(target,source) if (!source) {source = FastMPCW (); if (CClt) {pc += source;} else { pc += 2;}} else {if (CClt) pc += source;};
#define DoBlts(target,source) if (CClt) pc += source;
#define DoBgt(target,source) if (!source) {source = FastMPCW (); if (CCgt) {pc += source;} else { pc += 2;}} else {if (CCgt) pc += source;};
#define DoBgts(target,source) if (CCgt) pc += source;
#define DoBle(target,source) if (!source) {source = FastMPCW (); if (CCle) {pc += source;} else { pc += 2;}} else {if (CCle) pc += source;};
#define DoBles(target,source) if (CCle) pc += source;

#define DoDbt(target,source) if (!CCt) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbf(target,source) if (!CCf) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbhi(target,source) if (!CChi) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbls(target,source) if (!CCls) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbcc(target,source) if (!CCcc) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbcs(target,source) if (!CCcs) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbne(target,source) if (!CCne) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbeq(target,source) if (!CCeq) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbvc(target,source) if (!CCvc) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbvs(target,source) if (!CCvs) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbpl(target,source) if (!CCpl) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbmi(target,source) if (!CCmi) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbge(target,source) if (!CCge) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDblt(target,source) if (!CClt) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDbgt(target,source) if (!CCgt) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;
#define DoDble(target,source) if (!CCle) {if (--target != -1) pc += FastMPCW(); else pc += 2;} else pc += 2;

#define DoJmp(target,source) SetPC (source);
#define DoJsr(target,source) PushL(GetPC()); SetPC(source);
#define DoLea(target,source) target = source;
#define DoMovecc(target,source) target = source;
#define DoMovep(target,source) target = source;

#define DoMoveFromSR(target,source) \
    if (!GetS()) ExceptionGroup1(PRIV); \
target = source;

#define DoPea(target,source) PushL(source);


/* EXG */
OperBeginS(Opc140)
    register long var;
    var = cpup->reg[0];
    cpup->reg[0] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc148)
    register long var;
    var = cpup->reg[8];
    cpup->reg[8] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc188)
    register long var;
    var = cpup->reg[0];
    cpup->reg[0] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc340)
    register long var;
    var = cpup->reg[1];
    cpup->reg[1] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc348)
    register long var;
    var = cpup->reg[9];
    cpup->reg[9] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc388)
    register long var;
    var = cpup->reg[1];
    cpup->reg[1] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);


OperBeginS(Opc540)
    register long var;
    var = cpup->reg[2];
    cpup->reg[2] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc548)
    register long var;
    var = cpup->reg[10];
    cpup->reg[10] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc588)
    register long var;
    var = cpup->reg[2];
    cpup->reg[2] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc740)
    register long var;
    var = cpup->reg[3];
    cpup->reg[3] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc748)
    register long var;
    var = cpup->reg[11];
    cpup->reg[11] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc788)
    register long var;
    var = cpup->reg[3];
    cpup->reg[3] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc940)
    register long var;
    var = cpup->reg[4];
    cpup->reg[4] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc948)
    register long var;
    var = cpup->reg[12];
    cpup->reg[12] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opc988)
    register long var;
    var = cpup->reg[4];
    cpup->reg[4] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcb40)
    register long var;
    var = cpup->reg[5];
    cpup->reg[5] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcb48)
    register long var;
    var = cpup->reg[13];
    cpup->reg[13] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcb88)
    register long var;
    var = cpup->reg[5];
    cpup->reg[5] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcd40)
    register long var;
    var = cpup->reg[6];
    cpup->reg[6] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcd48)
    register long var;
    var = cpup->reg[14];
    cpup->reg[14] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcd88)
    register long var;
    var = cpup->reg[6];
    cpup->reg[6] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcf40)
    register long var;
    var = cpup->reg[7];
    cpup->reg[7] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcf48)
    register long var;
    var = cpup->reg[15];
    cpup->reg[15] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Opcf88)
    register long var;
    var = cpup->reg[7];
    cpup->reg[7] = cpup->reg[ins15];
    cpup->reg[ins15] = var;
    OperExitS(6);

OperBeginS(Op4848)
    ExceptionGroup1(ILLINSTR);
    OperExitS(4);


#define DoMoveMToP(siz, typ, rop, iop) { \
    address &= MEMADDRMASK; \
    if (address < IOBASE) { \
        register typ* addr = (typ*) (membase + address); \
        if (source & 0x0001) { *(addr)++ = rop(0); ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { *(addr)++ = rop(1); ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { *(addr)++ = rop(2); ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { *(addr)++ = rop(3); ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { *(addr)++ = rop(4); ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { *(addr)++ = rop(5); ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { *(addr)++ = rop(6); ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { *(addr)++ = rop(7); ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { *(addr)++ = rop(8); ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { *(addr)++ = rop(9); ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { *(addr)++ = rop(10); ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { *(addr)++ = rop(11); ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { *(addr)++ = rop(12); ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { *(addr)++ = rop(13); ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { *(addr)++ = rop(14); ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { *(addr)++ = rop(15); ADD_CYCLES(2*siz); }\
    } else { \
        if (source & 0x0001) { iop(address, rop(0)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { iop(address, rop(1)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { iop(address, rop(2)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { iop(address, rop(3)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { iop(address, rop(4)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { iop(address, rop(5)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { iop(address, rop(6)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { iop(address, rop(7)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { iop(address, rop(8)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { iop(address, rop(9)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { iop(address, rop(10)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { iop(address, rop(11)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { iop(address, rop(12)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { iop(address, rop(13)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { iop(address, rop(14)); address += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { iop(address, rop(15)); address += siz; ADD_CYCLES(2*siz); }\
    } \
}

#define DoMoveMToM(siz,typ,rop,iop) { \
    uint32 maddr = address & MEMADDRMASK; \
    if (maddr < IOBASE) { \
        register uint32 start = (uint32)(membase + maddr);\
        register typ* addr = (typ*) start;\
        if (source & 0x0001) { *(--addr) = rop(15); ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { *(--addr) = rop(14); ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { *(--addr) = rop(13); ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { *(--addr) = rop(12); ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { *(--addr) = rop(11); ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { *(--addr) = rop(10); ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { *(--addr) = rop(9); ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { *(--addr) = rop(8); ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { *(--addr) = rop(7); ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { *(--addr) = rop(6); ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { *(--addr) = rop(5); ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { *(--addr) = rop(4); ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { *(--addr) = rop(3); ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { *(--addr) = rop(2); ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { *(--addr) = rop(1); ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { *(--addr) = rop(0); ADD_CYCLES(2*siz); }\
        address -= (start - (uint32)addr); \
    } else { \
        register uint32 start = maddr; \
        if (source & 0x0001) { maddr-=siz; iop(maddr, rop(15)); ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { maddr-=siz; iop(maddr, rop(14)); ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { maddr-=siz; iop(maddr, rop(13)); ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { maddr-=siz; iop(maddr, rop(12)); ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { maddr-=siz; iop(maddr, rop(11)); ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { maddr-=siz; iop(maddr, rop(10)); ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { maddr-=siz; iop(maddr, rop(9)); ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { maddr-=siz; iop(maddr, rop(8)); ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { maddr-=siz; iop(maddr, rop(7)); ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { maddr-=siz; iop(maddr, rop(6)); ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { maddr-=siz; iop(maddr, rop(5)); ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { maddr-=siz; iop(maddr, rop(4)); ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { maddr-=siz; iop(maddr, rop(3)); ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { maddr-=siz; iop(maddr, rop(2)); ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { maddr-=siz; iop(maddr, rop(1)); ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { maddr-=siz; iop(maddr, rop(0)); ADD_CYCLES(2*siz); }\
        address -= (start - maddr);\
    } \
}

#define DoMoveMFrom(siz, typ, rop, iop) { \
    uint32 maddr = address & MEMADDRMASK; \
    if (maddr < IOBASE) { \
        register uint32 start = (uint32)(maddr + membase); \
        register typ* addr = (typ*) start; \
        if (source & 0x0001) { rop(0,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { rop(1,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { rop(2,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { rop(3,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { rop(4,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { rop(5,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { rop(6,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { rop(7,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { rop(8,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { rop(9,  (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { rop(10, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { rop(11, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { rop(12, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { rop(13, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { rop(14, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { rop(15, (int32) *(addr)++); ADD_CYCLES(2*siz); }\
        address += (((uint32)addr) - start); \
    } else { \
        register uint32 start = maddr; \
        if (source & 0x0001) { rop(0,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0002) { rop(1,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0004) { rop(2,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0008) { rop(3,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0010) { rop(4,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0020) { rop(5,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0040) { rop(6,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0080) { rop(7,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0100) { rop(8,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0200) { rop(9,  (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0400) { rop(10, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x0800) { rop(11, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x1000) { rop(12, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x2000) { rop(13, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x4000) { rop(14, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        if (source & 0x8000) { rop(15, (int32) iop(maddr)); maddr += siz; ADD_CYCLES(2*siz); }\
        address += (maddr - start); \
    } \
}


/* MOVEM.W to (ax) */
OperBegin(Op4890)
	//register unsigned long cycles=8+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMToP(2, uint16, GetRegW, DoIOWW);
    OperExit(8);
	//return cycles&0xfffffffc;

/* TODO: if ax is written, use initial ax content (68000, 68010)
*       if ax is written, use final ax content (other CPUs) */
/* MOVEM.W to -(ax) */
OperBegin(Op48a0)
	//register unsigned long cycles=8+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address = cpup->reg[8 + ins7];
    DoMoveMToM(2, uint16, GetRegW, DoIOWW);
    SetRegL(8 + ins7, address);
    OperExit(8);
	//return cycles&0xfffffffc;

/* MOVEM.W to d(ax) */
OperBegin(Op48a8)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cdai (address, ins7);
    DoMoveMToP(2, uint16, GetRegW, DoIOWW);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.W to d(ax,rx) */
OperBegin(Op48b0)
	//register unsigned long cycles=14+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Caix (address, ins7);
    DoMoveMToP(2, uint16, GetRegW, DoIOWW);
    OperExit(14);
	//return cycles&0xfffffffc;

/* MOVEM.W to w l */
OperBegin(Op48b8)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Ceaw (address, ins7);
    DoMoveMToP(2, uint16, GetRegW, DoIOWW);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.L to (ax) */
OperBegin(Op48d0)
	//register unsigned long cycles=8+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMToP(4, int32, GetRegL, DoIOWL);
    OperExit(8);
	//return cycles&0xfffffffc;

/* TODO: if ax is written, use initial ax content (68000, 68010)
*       if ax is written, use final ax content (other CPUs) */
/* MOVEM.L to -(ax) */
OperBegin(Op48e0)
	//register unsigned long cycles=8+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address = cpup->reg[8 + ins7];
    DoMoveMToM(4, uint32, GetRegL, DoIOWL);
    SetRegL(8 + ins7, address);
    OperExit(8);
	//return cycles&0xfffffffc;

/* MOVEM.L to d(ax) */
OperBegin(Op48e8)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cdai (address, ins7);
    DoMoveMToP(4, int32, GetRegL, DoIOWL);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.L to d(ax,rx) */
OperBegin(Op48f0)
	//register unsigned long cycles=14+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Caix (address, ins7);
    DoMoveMToP(4, int32, GetRegL, DoIOWL);
    OperExit(14);
	//return cycles&0xfffffffc;

/* MOVEM.L to w l */
OperBegin(Op48f8)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Ceaw (address, ins7);
    DoMoveMToP(4, int32, GetRegL, DoIOWL);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.W from (ax) */
OperBegin(Op4c90)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMFrom(2, int16, SetRegL, DoIORW);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.W from (ax)+ */
OperBegin(Op4c98)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMFrom(2, int16, SetRegL, DoIORW);
    SetRegL (8 + ins7, address);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.W from d(ax) */
OperBegin(Op4ca8)
	//register unsigned long cycles=16+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cdai (address, ins7);
    DoMoveMFrom(2, int16, SetRegL, DoIORW);
    OperExit(16);
	//return cycles&0xfffffffc;

/* MOVEM.W from d(ax,rx) */
OperBegin(Op4cb0)
	//register unsigned long cycles=18+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Caix (address, ins7);
    DoMoveMFrom(2, int16, SetRegL, DoIORW);
    OperExit(18);
	//return cycles&0xfffffffc;

/* MOVEM.W from ea */
OperBegin(Op4cb8)
	//register unsigned long cycles=16+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cear (address, ins7);
    DoMoveMFrom(2, int16, SetRegL, DoIORW);
    OperExit(16);
	//return cycles&0xfffffffc;

/* MOVEM.L from (ax) */
OperBegin(Op4cd0)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMFrom(4, int32, SetRegL, DoIORL);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.L from (ax)+ */
OperBegin(Op4cd8)
	//register unsigned long cycles=12+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cain (address, ins7);
    DoMoveMFrom(4, int32, SetRegL, DoIORL);
    SetRegL (8 + ins7, address);
    OperExit(12);
	//return cycles&0xfffffffc;

/* MOVEM.L from d(ax) */
OperBegin(Op4ce8)
	//register unsigned long cycles=16+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cdai (address, ins7);
    DoMoveMFrom(4, int32, SetRegL, DoIORL);
    OperExit(16);
	//return cycles&0xfffffffc;

/* MOVEM.L from d(ax,rx) */
OperBegin(Op4cf0)
	//register unsigned long cycles=18+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Caix (address, ins7);
    DoMoveMFrom(4, int32, SetRegL, DoIORL);
    OperExit(18);
	//return cycles&0xfffffffc;

/* MOVEM.L from ea */
OperBegin(Op4cf8)
	//register unsigned long cycles=16+3;
    uint16 source = GetMPCW(); pc+= 2;
    uint32 address;
    Cear (address, ins7);
    DoMoveMFrom(4, int32, SetRegL, DoIORL);
    OperExit(16);
	//return cycles&0xfffffffc;


/* TRAP */
OperBeginS(Op4e40)
    ExceptionGroup2(TRAP0);
    OperExitS(4);   // cycles?
OperBeginS(Op4e41)
    ExceptionGroup2(TRAP1);
    OperExitS(4);   // cycles?
OperBeginS(Op4e42)
    ExceptionGroup2(TRAP2);
    OperExitS(4);   // cycles?
OperBeginS(Op4e43)
    ExceptionGroup2(TRAP3);
    OperExitS(4);   // cycles?
OperBeginS(Op4e44)
    ExceptionGroup2(TRAP4);
    OperExitS(4);   // cycles?
OperBeginS(Op4e45)
    ExceptionGroup2(TRAP5);
    OperExitS(4);   // cycles?
OperBeginS(Op4e46)
    ExceptionGroup2(TRAP6);
    OperExitS(4);   // cycles?
OperBeginS(Op4e47)
    ExceptionGroup2(TRAP7);
    OperExitS(4);   // cycles?
OperBeginS(Op4e48)
    ExceptionGroup2(TRAP8);
    OperExitS(4);   // cycles?
OperBeginS(Op4e49)
    ExceptionGroup2(TRAP9);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4a)
    ExceptionGroup2(TRAP10);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4b)
    ExceptionGroup2(TRAP11);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4c)
    ExceptionGroup2(TRAP12);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4d)
    ExceptionGroup2(TRAP13);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4e)
    ExceptionGroup2(TRAP14);
    OperExitS(4);   // cycles?
OperBeginS(Op4e4f)
    ExceptionGroup2(TRAP15);
    OperExitS(4);   // cycles?

/* LINK */
OperBegin(Op4e50)
    long            source;
    source = GetRegL (8 + ins7);
    PushL(source);
    source = GetRegL (15);
    SetRegL (8 + ins7, source);
    source += GetMPCW ();
    pc += 2;
    SetRegL (15, source);
    OperExit(4);    // cycles?

/* UNLK */
OperBegin(Op4e58)
    long            source;
    source = GetRegL (8 + ins7);
    SetRegL (15, source);
    source = PopL();
    SetRegL (8 + ins7, source);
    OperExit(4);    // cycles?

/* MOVE ax,usp */
OperBeginS(Op4e60)
    if (!GetS ())
		ExceptionGroup1(PRIV);
    cpup->usp = cpup->reg[8 + (ins7)];
    OperExitS(4);   // cycles?

/* MOVE cpup->usp,ax */
OperBeginS(Op4e68)
    if (!GetS ())
		ExceptionGroup1(PRIV);
    cpup->reg[8 + (ins7)] = cpup->usp;
    OperExitS(4);   // cycles?

/* RESET, NOP, STOP, RTE, RTD, RTS, TRAPV, RTR */
OperBegin(Op4e70)
    switch (ins7) {
    case 0:         /* RESET */
        if (!GetS ())
            ExceptionGroup1(PRIV);
        break;
    case 1:         /* NOP */
        //__asm__ volatile ( "nop\n\t" : : : );
        ADDI_CYCLES(4);
        break;
    case 2:         /* STOP */
        Stop();
        break;
    case 3:         /* RTE */
        if (!GetS ())
            ExceptionGroup1(PRIV);
        {
            unsigned short  sr;
			psg_reg_vol=psg_cur_reg_vol;
#if CPU_TYPE == 68000
            sr = GetMemW (cpup->reg[15]); cpup->reg[15] += 2;
			if (cpup->state == -2){
				SetPC (PopL()+4);
				cpup->state =-1;
			} else {
                SetPC (PopL());
            }
            SetSRW(sr);
#else
            switch (GetMemW(cpup->reg[15] + 6) >> 12) {
            case 0x0:
                break;
            case 0x8:
                /* FIXME: this shouldn't be a format error */
#ifdef DEBUG
                DBG_STOP();
#endif
            default:
                ExceptionGroup1(FORMATERR);
                break;
            }
            sr = PopW();
            SetPC(PopL());
            SetSRW(sr);
            cpup->reg[15] += 2;
#endif
        }
        ADDI_CYCLES(8);
        break;
    case 4:         /* RTD */
#if CPU_TYPE == 68000
        ExceptionGroup1(ILLINSTR);
#else
        {
            short displacement = GetMPCW();
            SetPC(GetMemL(cpup->reg[15]));
            cpup->reg[15] = cpup->reg[15] + 4 + displacement;
        }
#endif
        break;
    case 5:         /* RTS */
        SetPC (PopL());
        break;
    case 6:         /* TRAPV */
        if (GetV ())
            ExceptionGroup1_TRAPV();
        break;
    case 7:         /* RTR */
        SetSRB (PopW());
        SetPC (PopL());
        break;
    }
    OperExit(4);        // cycles?


/* MOVEC */
OperBegin(Op4e78)
    switch (ins7) {
    case 2:
        if (!GetS ()) {
            ExceptionGroup1(PRIV);
        } else {
            short           source;
            int             index;
            source = GetMPCW ();
            pc += 2;
            index = source >> 12;
            switch (source & 0xfff) {
            case 0x000:
                cpup->reg[index] = cpup->sfc & 7;
                break;
            case 0x001:
                cpup->reg[index] = cpup->dfc & 7;
                break;
            case 0x800:
                cpup->reg[index] = cpup->usp;
                break;
            case 0x801:
                cpup->reg[index] = cpup->vbr;
                break;
#if CPU_TYPE == 68020
            case 0x002:
                cpup->reg[index] = cacr & 0x3;
                break;
            case 0x802:
                cpup->reg[index] = caar & 0xfc;
                break;
            case 0x803:
                cpup->reg[index] = msp;
                break;
            case 0x804:
                cpup->reg[index] = isp;
                break;
#endif
            default:
                /* as specified by Motorola */
                ExceptionGroup1(ILLINSTR);
                break;
            }
        }
        break;
    case 3:
        if (!GetS ()) {
            ExceptionGroup1(PRIV);
        } else {
            short           source;
            int             index;
            source = GetMPCW ();
            pc += 2;
            index = source >> 12;
            switch (source & 0xfff) {
            case 0x000:
                cpup->sfc = cpup->reg[index] & 7;
                break;
            case 0x001:
                cpup->dfc = cpup->reg[index] & 7;
                break;
            case 0x800:
                cpup->usp = cpup->reg[index];
                break;
            case 0x801:
                cpup->vbr = cpup->reg[index];
                break;
#if CPU_TYPE == 68020
            case 0x002:
                cacr = cpup->reg[index] & 0x3;
                break;
            case 0x802:
                caar = cpup->reg[index] & 0xfc;
                break;
            case 0x803:
                msp = cpup->reg[index];
                break;
            case 0x804:
                isp = cpup->reg[index];
                break;
#endif
            default:
                /* as specified by Motorola */
                ExceptionGroup1(ILLINSTR);
                break;
            }
        }
        break;
    default:
        ExceptionGroup1(ILLINSTR);
        break;
    }
    OperExit(4);    // cycles?

#else

#define OMoves(a1,a2,a3,a4,a5,a6,a7,a8) \
extern void a1();
#define OMoveToSR(a1, a2, a3)\
extern void a1();
#endif

OperS(Op4000, DoNegxB, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 4)
Oper (Op4010, DoNegxB, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op4018, DoNegxB, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op4020, DoNegxB, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4028, DoNegxB, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op4030, DoNegxB, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4038, DoNegxB, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 18)
OperS(Op4040, DoNegxW, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 4)
Oper (Op4050, DoNegxW, DN, SNN, 0, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Op4058, DoNegxW, DN, SNN, 0, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Op4060, DoNegxW, DN, SNN, 0, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Op4068, DoNegxW, DN, SNN, 0, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Op4070, DoNegxW, DN, SNN, 0, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Op4078, DoNegxW, DN, SNN, 0, DW, DA, Feaw, ins7, GMW, SMW, 18)
OperS(Op4080, DoNegxL, DN, SNN, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op4090, DoNegxL, DN, SNN, 0, DL, DA, Cain, ins7, GML, SML, 20)
Oper (Op4098, DoNegxL, DN, SNN, 0, DL, DA, CaipL, ins7, GML, SML, 20)
Oper (Op40a0, DoNegxL, DN, SNN, 0, DL, DA, CmaiL, ins7, GML, SML, 24)
Oper (Op40a8, DoNegxL, DN, SNN, 0, DL, DA, Fdai, ins7, GML, SML, 24)
Oper (Op40b0, DoNegxL, DN, SNN, 0, DL, DA, Faix, ins7, GML, SML, 28)
Oper (Op40b8, DoNegxL, DN, SNN, 0, DL, DA, Feaw, ins7, GML, SML, 26)
#if CPU_TYPE == 68000 // 68000: MOVE from SR not privileged
OperS(Op40c0, DoMovecc, DW, ScW, 0, DW, DR, Cd, ins7, GN, SRW, 8)
Oper (Op40d0, DoMovecc, DW, ScW, 0, DW, DA, Cain, ins7, GN, SMW, 12)
Oper (Op40d8, DoMovecc, DW, ScW, 0, DW, DA, CaipW, ins7, GN, SMW, 12)
Oper (Op40e0, DoMovecc, DW, ScW, 0, DW, DA, CmaiW, ins7, GN, SMW, 16)
Oper (Op40e8, DoMovecc, DW, ScW, 0, DW, DA, Fdai, ins7, GN, SMW, 16)
Oper (Op40f0, DoMovecc, DW, ScW, 0, DW, DA, Faix, ins7, GN, SMW, 20)
Oper (Op40f8, DoMovecc, DW, ScW, 0, DW, DA, Feaw, ins7, GN, SMW, 18)
#else
Oper (Op40c0, DoMoveFromSR, DW, ScW, 0, DW, DR, Cd, ins7, GN, SRW, 8)
Oper (Op40d0, DoMoveFromSR, DW, ScW, 0, DW, DA, Cain, ins7, GN, SMW, 12)
Oper (Op40d8, DoMoveFromSR, DW, ScW, 0, DW, DA, CaipW, ins7, GN, SMW, 12)
Oper (Op40e0, DoMoveFromSR, DW, ScW, 0, DW, DA, CmaiW, ins7, GN, SMW, 16)
Oper (Op40e8, DoMoveFromSR, DW, ScW, 0, DW, DA, Fdai, ins7, GN, SMW, 16)
Oper (Op40f0, DoMoveFromSR, DW, ScW, 0, DW, DA, Faix, ins7, GN, SMW, 20)
Oper (Op40f8, DoMoveFromSR, DW, ScW, 0, DW, DA, Feaw, ins7, GN, SMW, 18)
#endif

OperS(Op4200, DoClr, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 4)
Oper (Op4210, DoClr, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op4218, DoClr, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op4220, DoClr, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4228, DoClr, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op4230, DoClr, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4238, DoClr, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 18)
OperS(Op4240, DoClr, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 4)
Oper (Op4250, DoClr, DN, SNN, 0, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Op4258, DoClr, DN, SNN, 0, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Op4260, DoClr, DN, SNN, 0, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Op4268, DoClr, DN, SNN, 0, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Op4270, DoClr, DN, SNN, 0, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Op4278, DoClr, DN, SNN, 0, DW, DA, Feaw, ins7, GMW, SMW, 18)
OperS(Op4280, DoClr, DN, SNN, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op4290, DoClr, DN, SNN, 0, DL, DA, Cain, ins7, GML, SML, 20)
Oper (Op4298, DoClr, DN, SNN, 0, DL, DA, CaipL, ins7, GML, SML, 20)
Oper (Op42a0, DoClr, DN, SNN, 0, DL, DA, CmaiL, ins7, GML, SML, 24)
Oper (Op42a8, DoClr, DN, SNN, 0, DL, DA, Fdai, ins7, GML, SML, 24)
Oper (Op42b0, DoClr, DN, SNN, 0, DL, DA, Faix, ins7, GML, SML, 28)
Oper (Op42b8, DoClr, DN, SNN, 0, DL, DA, Feaw, ins7, GML, SML, 26)
OperS(Op42c0, DoMovecc, DW, ScB, 0, DW, DR, Cd, ins7, GN, SRW, 8)
Oper (Op42d0, DoMovecc, DW, ScB, 0, DW, DA, Cain, ins7, GN, SMW, 12)
Oper (Op42d8, DoMovecc, DW, ScB, 0, DW, DA, CaipW, ins7, GN, SMW, 12)
Oper (Op42e0, DoMovecc, DW, ScB, 0, DW, DA, CmaiW, ins7, GN, SMW, 16)
Oper (Op42e8, DoMovecc, DW, ScB, 0, DW, DA, Fdai, ins7, GN, SMW, 16)
Oper (Op42f0, DoMovecc, DW, ScB, 0, DW, DA, Faix, ins7, GN, SMW, 20)
Oper (Op42f8, DoMovecc, DW, ScB, 0, DW, DA, Feaw, ins7, GN, SMW, 18)

OperS(Op4400, DoNegB, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 4)
Oper (Op4410, DoNegB, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op4418, DoNegB, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op4420, DoNegB, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4428, DoNegB, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op4430, DoNegB, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4438, DoNegB, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 18)
OperS(Op4440, DoNegW, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 4)
Oper (Op4450, DoNegW, DN, SNN, 0, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Op4458, DoNegW, DN, SNN, 0, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Op4460, DoNegW, DN, SNN, 0, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Op4468, DoNegW, DN, SNN, 0, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Op4470, DoNegW, DN, SNN, 0, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Op4478, DoNegW, DN, SNN, 0, DW, DA, Feaw, ins7, GMW, SMW, 18)
OperS(Op4480, DoNegL, DN, SNN, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op4490, DoNegL, DN, SNN, 0, DL, DA, Cain, ins7, GML, SML, 20)
Oper (Op4498, DoNegL, DN, SNN, 0, DL, DA, CaipL, ins7, GML, SML, 20)
Oper (Op44a0, DoNegL, DN, SNN, 0, DL, DA, CmaiL, ins7, GML, SML, 24)
Oper (Op44a8, DoNegL, DN, SNN, 0, DL, DA, Fdai, ins7, GML, SML, 24)
Oper (Op44b0, DoNegL, DN, SNN, 0, DL, DA, Faix, ins7, GML, SML, 28)
Oper (Op44b8, DoNegL, DN, SNN, 0, DL, DA, Feaw, ins7, GML, SML, 26)
OperS(Op44c0, DoMovecc, DW, SdW, ins7, DB, DN, CN, 0, GN, SCB, 12)
OperS(Op44d0, DoMovecc, DW, SainW, ins7, DB, DN, CN, 0, GN, SCB, 16)
OperS(Op44d8, DoMovecc, DW, SaipW, ins7, DB, DN, CN, 0, GN, SCB, 16)
OperS(Op44e0, DoMovecc, DW, SmaiW, ins7, DB, DN, CN, 0, GN, SCB, 20)
OperS(Op44e8, DoMovecc, DW, SdaiW, ins7, DB, DN, CN, 0, GN, SCB, 20)
Oper (Op44f0, DoMovecc, DW, SaixW, ins7, DB, DN, CN, 0, GN, SCB, 24)
OperS(Op44f8, DoMovecc, DW, SearW, ins7, DB, DN, CN, 0, GN, SCB, 18)

OperS(Op4600, DoNotB, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 4)
Oper (Op4610, DoNotB, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op4618, DoNotB, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op4620, DoNotB, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4628, DoNotB, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op4630, DoNotB, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4638, DoNotB, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 18)
OperS(Op4640, DoNotW, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 4)
Oper (Op4650, DoNotW, DN, SNN, 0, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Op4658, DoNotW, DN, SNN, 0, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Op4660, DoNotW, DN, SNN, 0, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Op4668, DoNotW, DN, SNN, 0, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Op4670, DoNotW, DN, SNN, 0, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Op4678, DoNotW, DN, SNN, 0, DW, DA, Feaw, ins7, GMW, SMW, 18)
OperS(Op4680, DoNotL, DN, SNN, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op4690, DoNotL, DN, SNN, 0, DL, DA, Cain, ins7, GML, SML, 20)
Oper (Op4698, DoNotL, DN, SNN, 0, DL, DA, CaipL, ins7, GML, SML, 20)
Oper (Op46a0, DoNotL, DN, SNN, 0, DL, DA, CmaiL, ins7, GML, SML, 24)
Oper (Op46a8, DoNotL, DN, SNN, 0, DL, DA, Fdai, ins7, GML, SML, 24)
Oper (Op46b0, DoNotL, DN, SNN, 0, DL, DA, Faix, ins7, GML, SML, 28)
Oper (Op46b8, DoNotL, DN, SNN, 0, DL, DA, Feaw, ins7, GML, SML, 26)
OMoveToSR (Op46c0, SdW, 12)
OMoveToSR (Op46d0, SainW, 16)
OMoveToSR (Op46d8, SaipW, 16)
OMoveToSR (Op46e0, SmaiW, 18)
OMoveToSR (Op46e8, SdaiW, 20)
OMoveToSR (Op46f0, SaixW, 22)
OMoveToSR (Op46f8, SearW, 18)

OperS(Op4850, DoPea, DL, SainA, ins7, DN, DN, CN, 0, GN, SN, 12)
OperS(Op4868, DoPea, DL, SdaiA, ins7, DN, DN, CN, 0, GN, SN, 16)
Oper (Op4870, DoPea, DL, SaixA, ins7, DN, DN, CN, 0, GN, SN, 20)
OperS(Op4878, DoPea, DL, SearA, ins7, DN, DN, CN, 0, GN, SN, 18)

OperS(Op4a00, DoTstB, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SN, 4)
OperS(Op4a10, DoTstB, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SN, 8)
OperS(Op4a18, DoTstB, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SN, 8)
OperS(Op4a20, DoTstB, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SN, 12)
OperS(Op4a28, DoTstB, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SN, 12)
OperS(Op4a30, DoTstB, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SN, 16)
OperS(Op4a38, DoTstB, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SN, 10)
OperS(Op4a40, DoTstW, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SN, 4)
OperS(Op4a50, DoTstW, DN, SNN, 0, DW, DA, Cain, ins7, GMW, SN, 8)
OperS(Op4a58, DoTstW, DN, SNN, 0, DW, DA, CaipW, ins7, GMW, SN, 8)
OperS(Op4a60, DoTstW, DN, SNN, 0, DW, DA, CmaiW, ins7, GMW, SN, 12)
OperS(Op4a68, DoTstW, DN, SNN, 0, DW, DA, Fdai, ins7, GMW, SN, 12)
OperS(Op4a70, DoTstW, DN, SNN, 0, DW, DA, Faix, ins7, GMW, SN, 16)
OperS(Op4a78, DoTstW, DN, SNN, 0, DW, DA, Feaw, ins7, GMW, SN, 10)
OperS(Op4a80, DoTstL, DN, SNN, 0, DL, DR, Cd, ins7, GRL, SN, 4)
OperS(Op4a90, DoTstL, DN, SNN, 0, DL, DA, Cain, ins7, GML, SN, 12)
OperS(Op4a98, DoTstL, DN, SNN, 0, DL, DA, CaipL, ins7, GML, SN, 12)
OperS(Op4aa0, DoTstL, DN, SNN, 0, DL, DA, CmaiL, ins7, GML, SN, 16)
OperS(Op4aa8, DoTstL, DN, SNN, 0, DL, DA, Fdai, ins7, GML, SN, 16)
OperS(Op4ab0, DoTstL, DN, SNN, 0, DL, DA, Faix, ins7, GML, SN, 20)
OperS(Op4ab8, DoTstL, DN, SNN, 0, DL, DA, Feaw, ins7, GML, SN, 14)
Oper (Op4ac0, DoTas, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 4)
Oper (Op4ad0, DoTas, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 16)
Oper (Op4ad8, DoTas, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 16)
Oper (Op4ae0, DoTas, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4ae8, DoTas, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 20)
Oper (Op4af0, DoTas, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4af8, DoTas, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 20)

Oper (Op4e90, DoJsr, DL, SainA, ins7, DN, DN, CN, 0, GN, SN, 16)
Oper (Op4ea8, DoJsr, DL, SdaiA, ins7, DN, DN, CN, 0, GN, SN, 20)
Oper (Op4eb0, DoJsr, DL, SaixA, ins7, DN, DN, CN, 0, GN, SN, 24)
Oper (Op4eb8, DoJsr, DL, SearA, ins7, DN, DN, CN, 0, GN, SN, 20)

Oper (Op4ed0, DoJmp, DL, SainA, ins7, DN, DN, CN, 0, GN, SN, 8)
Oper (Op4ee8, DoJmp, DL, SdaiA, ins7, DN, DN, CN, 0, GN, SN, 12)
Oper (Op4ef0, DoJmp, DL, SaixA, ins7, DN, DN, CN, 0, GN, SN, 16)
Oper (Op4ef8, DoJmp, DL, SearA, ins7, DN, DN, CN, 0, GN, SN, 12)

OperS(Op4180, DoChk, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4190, DoChk, DW, SdW, 0, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4198, DoChk, DW, SdW, 0, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op41a0, DoChk, DW, SdW, 0, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op41a8, DoChk, DW, SdW, 0, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op41b0, DoChk, DW, SdW, 0, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op41b8, DoChk, DW, SdW, 0, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4380, DoChk, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4390, DoChk, DW, SdW, 1, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4398, DoChk, DW, SdW, 1, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op43a0, DoChk, DW, SdW, 1, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op43a8, DoChk, DW, SdW, 1, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op43b0, DoChk, DW, SdW, 1, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op43b8, DoChk, DW, SdW, 1, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4580, DoChk, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4590, DoChk, DW, SdW, 2, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4598, DoChk, DW, SdW, 2, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op45a0, DoChk, DW, SdW, 2, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op45a8, DoChk, DW, SdW, 2, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op45b0, DoChk, DW, SdW, 2, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op45b8, DoChk, DW, SdW, 2, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4780, DoChk, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4790, DoChk, DW, SdW, 3, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4798, DoChk, DW, SdW, 3, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op47a0, DoChk, DW, SdW, 3, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op47a8, DoChk, DW, SdW, 3, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op47b0, DoChk, DW, SdW, 3, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op47b8, DoChk, DW, SdW, 3, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4980, DoChk, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4990, DoChk, DW, SdW, 4, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4998, DoChk, DW, SdW, 4, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op49a0, DoChk, DW, SdW, 4, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op49a8, DoChk, DW, SdW, 4, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op49b0, DoChk, DW, SdW, 4, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op49b8, DoChk, DW, SdW, 4, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4b80, DoChk, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4b90, DoChk, DW, SdW, 5, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4b98, DoChk, DW, SdW, 5, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op4ba0, DoChk, DW, SdW, 5, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op4ba8, DoChk, DW, SdW, 5, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op4bb0, DoChk, DW, SdW, 5, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op4bb8, DoChk, DW, SdW, 5, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4d80, DoChk, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4d90, DoChk, DW, SdW, 6, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4d98, DoChk, DW, SdW, 6, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op4da0, DoChk, DW, SdW, 6, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op4da8, DoChk, DW, SdW, 6, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op4db0, DoChk, DW, SdW, 6, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op4db8, DoChk, DW, SdW, 6, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op4f80, DoChk, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SN, 12)
Oper (Op4f90, DoChk, DW, SdW, 7, DW, DA, Cain, ins7, GMW, SN, 16)
Oper (Op4f98, DoChk, DW, SdW, 7, DW, DA, CaipB, ins7, GMW, SN, 16)
Oper (Op4fa0, DoChk, DW, SdW, 7, DW, DA, CmaiB, ins7, GMW, SN, 16)
Oper (Op4fa8, DoChk, DW, SdW, 7, DW, DA, Fdai, ins7, GMW, SN, 20)
Oper (Op4fb0, DoChk, DW, SdW, 7, DW, DA, Faix, ins7, GMW, SN, 20)
Oper (Op4fb8, DoChk, DW, SdW, 7, DW, DA, Fear, ins7, GMW, SN, 16)

OperS(Op41d0, DoLea, DL, SainA, ins7, DL, DR, Ca, 0, GN, SRL, 4)
OperS(Op41e8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 0, GN, SRL, 8)
OperS(Op41f0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 0, GN, SRL, 12)
OperS(Op41f8, DoLea, DL, SearA, ins7, DL, DR, Ca, 0, GN, SRL, 10)
OperS(Op43d0, DoLea, DL, SainA, ins7, DL, DR, Ca, 1, GN, SRL, 4)
OperS(Op43e8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 1, GN, SRL, 8)
OperS(Op43f0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 1, GN, SRL, 12)
OperS(Op43f8, DoLea, DL, SearA, ins7, DL, DR, Ca, 1, GN, SRL, 10)
OperS(Op45d0, DoLea, DL, SainA, ins7, DL, DR, Ca, 2, GN, SRL, 4)
OperS(Op45e8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 2, GN, SRL, 8)
OperS(Op45f0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 2, GN, SRL, 12)
OperS(Op45f8, DoLea, DL, SearA, ins7, DL, DR, Ca, 2, GN, SRL, 10)
OperS(Op47d0, DoLea, DL, SainA, ins7, DL, DR, Ca, 3, GN, SRL, 4)
OperS(Op47e8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 3, GN, SRL, 8)
OperS(Op47f0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 3, GN, SRL, 12)
OperS(Op47f8, DoLea, DL, SearA, ins7, DL, DR, Ca, 3, GN, SRL, 10)
OperS(Op49d0, DoLea, DL, SainA, ins7, DL, DR, Ca, 4, GN, SRL, 4)
OperS(Op49e8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 4, GN, SRL, 8)
OperS(Op49f0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 4, GN, SRL, 12)
OperS(Op49f8, DoLea, DL, SearA, ins7, DL, DR, Ca, 4, GN, SRL, 10)
OperS(Op4bd0, DoLea, DL, SainA, ins7, DL, DR, Ca, 5, GN, SRL, 4)
OperS(Op4be8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 5, GN, SRL, 8)
OperS(Op4bf0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 5, GN, SRL, 12)
OperS(Op4bf8, DoLea, DL, SearA, ins7, DL, DR, Ca, 5, GN, SRL, 10)
OperS(Op4dd0, DoLea, DL, SainA, ins7, DL, DR, Ca, 6, GN, SRL, 4)
OperS(Op4de8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 6, GN, SRL, 8)
OperS(Op4df0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 6, GN, SRL, 12)
OperS(Op4df8, DoLea, DL, SearA, ins7, DL, DR, Ca, 6, GN, SRL, 10)
OperS(Op4fd0, DoLea, DL, SainA, ins7, DL, DR, Ca, 7, GN, SRL, 4)
OperS(Op4fe8, DoLea, DL, SdaiA, ins7, DL, DR, Ca, 7, GN, SRL, 8)
OperS(Op4ff0, DoLea, DL, SaixA, ins7, DL, DR, Ca, 7, GN, SRL, 12)
OperS(Op4ff8, DoLea, DL, SearA, ins7, DL, DR, Ca, 7, GN, SRL, 10)

OperS(Op50c0, DoSt, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op50c8, DoDbt, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op50d0, DoSt, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op50d8, DoSt, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op50e0, DoSt, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op50e8, DoSt, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op50f0, DoSt, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op50f8, DoSt, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op51c0, DoSf, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op51c8, DoDbf, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op51d0, DoSf, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op51d8, DoSf, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op51e0, DoSf, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op51e8, DoSf, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op51f0, DoSf, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op51f8, DoSf, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op52c0, DoShi, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op52c8, DoDbhi, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op52d0, DoShi, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op52d8, DoShi, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op52e0, DoShi, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op52e8, DoShi, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op52f0, DoShi, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op52f8, DoShi, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op53c0, DoSls, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op53c8, DoDbls, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op53d0, DoSls, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op53d8, DoSls, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op53e0, DoSls, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op53e8, DoSls, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op53f0, DoSls, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op53f8, DoSls, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op54c0, DoScc, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op54c8, DoDbcc, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op54d0, DoScc, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op54d8, DoScc, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op54e0, DoScc, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op54e8, DoScc, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op54f0, DoScc, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op54f8, DoScc, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op55c0, DoScs, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op55c8, DoDbcs, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op55d0, DoScs, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op55d8, DoScs, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op55e0, DoScs, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op55e8, DoScs, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op55f0, DoScs, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op55f8, DoScs, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op56c0, DoSne, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op56c8, DoDbne, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op56d0, DoSne, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op56d8, DoSne, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op56e0, DoSne, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op56e8, DoSne, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op56f0, DoSne, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op56f8, DoSne, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op57c0, DoSeq, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op57c8, DoDbeq, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op57d0, DoSeq, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op57d8, DoSeq, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op57e0, DoSeq, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op57e8, DoSeq, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op57f0, DoSeq, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op57f8, DoSeq, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op58c0, DoSvc, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op58c8, DoDbvc, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op58d0, DoSvc, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op58d8, DoSvc, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op58e0, DoSvc, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op58e8, DoSvc, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op58f0, DoSvc, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op58f8, DoSvc, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op59c0, DoSvs, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op59c8, DoDbvs, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op59d0, DoSvs, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op59d8, DoSvs, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op59e0, DoSvs, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op59e8, DoSvs, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op59f0, DoSvs, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op59f8, DoSvs, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5ac0, DoSpl, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5ac8, DoDbpl, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5ad0, DoSpl, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5ad8, DoSpl, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5ae0, DoSpl, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5ae8, DoSpl, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5af0, DoSpl, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5af8, DoSpl, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5bc0, DoSmi, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5bc8, DoDbmi, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5bd0, DoSmi, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5bd8, DoSmi, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5be0, DoSmi, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5be8, DoSmi, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5bf0, DoSmi, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5bf8, DoSmi, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5cc0, DoSge, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5cc8, DoDbge, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5cd0, DoSge, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5cd8, DoSge, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5ce0, DoSge, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5ce8, DoSge, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5cf0, DoSge, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5cf8, DoSge, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5dc0, DoSlt, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5dc8, DoDblt, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5dd0, DoSlt, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5dd8, DoSlt, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5de0, DoSlt, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5de8, DoSlt, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5df0, DoSlt, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5df8, DoSlt, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5ec0, DoSgt, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5ec8, DoDbgt, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5ed0, DoSgt, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5ed8, DoSgt, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5ee0, DoSgt, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5ee8, DoSgt, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5ef0, DoSgt, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5ef8, DoSgt, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op5fc0, DoSle, DN, SNN, 0, DB, DR, Cd, ins7, GN, SRB, 4)
OperS(Op5fc8, DoDble, DN, SNN, 0, DW, DR, Cd, ins7, GRW, SRW, 12)
Oper (Op5fd0, DoSle, DN, SNN, 0, DB, DA, Cain, ins7, GN, SMB, 12)
Oper (Op5fd8, DoSle, DN, SNN, 0, DB, DA, CaipB, ins7, GN, SMB, 12)
Oper (Op5fe0, DoSle, DN, SNN, 0, DB, DA, CmaiB, ins7, GN, SMB, 16)
Oper (Op5fe8, DoSle, DN, SNN, 0, DB, DA, Fdai, ins7, GN, SMB, 16)
Oper (Op5ff0, DoSle, DN, SNN, 0, DB, DA, Faix, ins7, GN, SMB, 20)
Oper (Op5ff8, DoSle, DN, SNN, 0, DB, DA, Feaw, ins7, GN, SMB, 18)

OperS(Op6000, DoBra, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6008, DoBras, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
Oper (Op6100, DoBsr, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 20)
Oper (Op6108, DoBsrs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6200, DoBhi, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6208, DoBhis, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6300, DoBls, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6308, DoBlss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6400, DoBcc, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6408, DoBccs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6500, DoBcs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6508, DoBcss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6600, DoBne, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6608, DoBnes, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6700, DoBeq, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6708, DoBeqs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6800, DoBvc, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6808, DoBvcs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6900, DoBvs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6908, DoBvss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6a00, DoBpl, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6a08, DoBpls, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6b00, DoBmi, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6b08, DoBmis, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6c00, DoBge, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6c08, DoBges, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6d00, DoBlt, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6d08, DoBlts, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6e00, DoBgt, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6e08, DoBgts, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 4)
OperS(Op6f00, DoBle, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)
OperS(Op6f08, DoBles, DW, SNQ, 0, DN, DN, CN, 0, GN, SN, 12)

OperS(Op0100, DoBtst32, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0108, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 0, GN, SRW, 16)
Oper (Op0110, DoBtst8, DB, SdB, 0, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0118, DoBtst8, DB, SdB, 0, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0120, DoBtst8, DB, SdB, 0, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0128, DoBtst8, DB, SdB, 0, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0130, DoBtst8, DB, SdB, 0, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0138, DoBtst8, DB, SdB, 0, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0140, DoBchg32, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0148, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 0, GN, SRL, 24)
Oper (Op0150, DoBchg8, DB, SdB, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0158, DoBchg8, DB, SdB, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0160, DoBchg8, DB, SdB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0168, DoBchg8, DB, SdB, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0170, DoBchg8, DB, SdB, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0178, DoBchg8, DB, SdB, 0, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0180, DoBclr32, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0188, DoMovep, DW, SdW, 0, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0190, DoBclr8, DB, SdB, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0198, DoBclr8, DB, SdB, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op01a0, DoBclr8, DB, SdB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op01a8, DoBclr8, DB, SdB, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op01b0, DoBclr8, DB, SdB, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op01b8, DoBclr8, DB, SdB, 0, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op01c0, DoBset32, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op01c8, DoMovep, DL, SdL, 0, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op01d0, DoBset8, DB, SdB, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op01d8, DoBset8, DB, SdB, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op01e0, DoBset8, DB, SdB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op01e8, DoBset8, DB, SdB, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op01f0, DoBset8, DB, SdB, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op01f8, DoBset8, DB, SdB, 0, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0300, DoBtst32, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0308, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 1, GN, SRW, 16)
Oper (Op0310, DoBtst8, DB, SdB, 1, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0318, DoBtst8, DB, SdB, 1, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0320, DoBtst8, DB, SdB, 1, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0328, DoBtst8, DB, SdB, 1, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0330, DoBtst8, DB, SdB, 1, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0338, DoBtst8, DB, SdB, 1, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0340, DoBchg32, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0348, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 1, GN, SRL, 24)
Oper (Op0350, DoBchg8, DB, SdB, 1, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0358, DoBchg8, DB, SdB, 1, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0360, DoBchg8, DB, SdB, 1, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0368, DoBchg8, DB, SdB, 1, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0370, DoBchg8, DB, SdB, 1, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0378, DoBchg8, DB, SdB, 1, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0380, DoBclr32, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0388, DoMovep, DW, SdW, 1, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0390, DoBclr8, DB, SdB, 1, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0398, DoBclr8, DB, SdB, 1, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op03a0, DoBclr8, DB, SdB, 1, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op03a8, DoBclr8, DB, SdB, 1, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op03b0, DoBclr8, DB, SdB, 1, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op03b8, DoBclr8, DB, SdB, 1, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op03c0, DoBset32, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op03c8, DoMovep, DL, SdL, 1, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op03d0, DoBset8, DB, SdB, 1, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op03d8, DoBset8, DB, SdB, 1, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op03e0, DoBset8, DB, SdB, 1, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op03e8, DoBset8, DB, SdB, 1, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op03f0, DoBset8, DB, SdB, 1, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op03f8, DoBset8, DB, SdB, 1, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0500, DoBtst32, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0508, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 2, GN, SRW, 16)
Oper (Op0510, DoBtst8, DB, SdB, 2, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0518, DoBtst8, DB, SdB, 2, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0520, DoBtst8, DB, SdB, 2, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0528, DoBtst8, DB, SdB, 2, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0530, DoBtst8, DB, SdB, 2, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0538, DoBtst8, DB, SdB, 2, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0540, DoBchg32, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0548, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 2, GN, SRL, 24)
Oper (Op0550, DoBchg8, DB, SdB, 2, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0558, DoBchg8, DB, SdB, 2, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0560, DoBchg8, DB, SdB, 2, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0568, DoBchg8, DB, SdB, 2, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0570, DoBchg8, DB, SdB, 2, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0578, DoBchg8, DB, SdB, 2, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0580, DoBclr32, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0588, DoMovep, DW, SdW, 2, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0590, DoBclr8, DB, SdB, 2, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0598, DoBclr8, DB, SdB, 2, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op05a0, DoBclr8, DB, SdB, 2, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op05a8, DoBclr8, DB, SdB, 2, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op05b0, DoBclr8, DB, SdB, 2, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op05b8, DoBclr8, DB, SdB, 2, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op05c0, DoBset32, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op05c8, DoMovep, DL, SdL, 2, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op05d0, DoBset8, DB, SdB, 2, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op05d8, DoBset8, DB, SdB, 2, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op05e0, DoBset8, DB, SdB, 2, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op05e8, DoBset8, DB, SdB, 2, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op05f0, DoBset8, DB, SdB, 2, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op05f8, DoBset8, DB, SdB, 2, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0700, DoBtst32, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0708, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 3, GN, SRW, 16)
Oper (Op0710, DoBtst8, DB, SdB, 3, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0718, DoBtst8, DB, SdB, 3, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0720, DoBtst8, DB, SdB, 3, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0728, DoBtst8, DB, SdB, 3, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0730, DoBtst8, DB, SdB, 3, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0738, DoBtst8, DB, SdB, 3, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0740, DoBchg32, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0748, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 3, GN, SRL, 24)
Oper (Op0750, DoBchg8, DB, SdB, 3, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0758, DoBchg8, DB, SdB, 3, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0760, DoBchg8, DB, SdB, 3, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0768, DoBchg8, DB, SdB, 3, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0770, DoBchg8, DB, SdB, 3, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0778, DoBchg8, DB, SdB, 3, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0780, DoBclr32, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0788, DoMovep, DW, SdW, 3, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0790, DoBclr8, DB, SdB, 3, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0798, DoBclr8, DB, SdB, 3, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op07a0, DoBclr8, DB, SdB, 3, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op07a8, DoBclr8, DB, SdB, 3, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op07b0, DoBclr8, DB, SdB, 3, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op07b8, DoBclr8, DB, SdB, 3, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op07c0, DoBset32, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op07c8, DoMovep, DL, SdL, 3, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op07d0, DoBset8, DB, SdB, 3, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op07d8, DoBset8, DB, SdB, 3, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op07e0, DoBset8, DB, SdB, 3, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op07e8, DoBset8, DB, SdB, 3, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op07f0, DoBset8, DB, SdB, 3, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op07f8, DoBset8, DB, SdB, 3, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0900, DoBtst32, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0908, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 4, GN, SRW, 16)
Oper (Op0910, DoBtst8, DB, SdB, 4, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0918, DoBtst8, DB, SdB, 4, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0920, DoBtst8, DB, SdB, 4, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0928, DoBtst8, DB, SdB, 4, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0930, DoBtst8, DB, SdB, 4, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0938, DoBtst8, DB, SdB, 4, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0940, DoBchg32, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0948, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 4, GN, SRL, 24)
Oper (Op0950, DoBchg8, DB, SdB, 4, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0958, DoBchg8, DB, SdB, 4, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0960, DoBchg8, DB, SdB, 4, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0968, DoBchg8, DB, SdB, 4, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0970, DoBchg8, DB, SdB, 4, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0978, DoBchg8, DB, SdB, 4, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0980, DoBclr32, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0988, DoMovep, DW, SdW, 4, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0990, DoBclr8, DB, SdB, 4, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0998, DoBclr8, DB, SdB, 4, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op09a0, DoBclr8, DB, SdB, 4, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op09a8, DoBclr8, DB, SdB, 4, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op09b0, DoBclr8, DB, SdB, 4, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op09b8, DoBclr8, DB, SdB, 4, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op09c0, DoBset32, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op09c8, DoMovep, DL, SdL, 4, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op09d0, DoBset8, DB, SdB, 4, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op09d8, DoBset8, DB, SdB, 4, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op09e0, DoBset8, DB, SdB, 4, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op09e8, DoBset8, DB, SdB, 4, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op09f0, DoBset8, DB, SdB, 4, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op09f8, DoBset8, DB, SdB, 4, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0b00, DoBtst32, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0b08, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 5, GN, SRW, 16)
Oper (Op0b10, DoBtst8, DB, SdB, 5, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0b18, DoBtst8, DB, SdB, 5, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0b20, DoBtst8, DB, SdB, 5, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0b28, DoBtst8, DB, SdB, 5, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0b30, DoBtst8, DB, SdB, 5, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0b38, DoBtst8, DB, SdB, 5, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0b40, DoBchg32, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0b48, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 5, GN, SRL, 24)
Oper (Op0b50, DoBchg8, DB, SdB, 5, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0b58, DoBchg8, DB, SdB, 5, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0b60, DoBchg8, DB, SdB, 5, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0b68, DoBchg8, DB, SdB, 5, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0b70, DoBchg8, DB, SdB, 5, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0b78, DoBchg8, DB, SdB, 5, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0b80, DoBclr32, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0b88, DoMovep, DW, SdW, 5, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0b90, DoBclr8, DB, SdB, 5, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0b98, DoBclr8, DB, SdB, 5, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0ba0, DoBclr8, DB, SdB, 5, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0ba8, DoBclr8, DB, SdB, 5, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0bb0, DoBclr8, DB, SdB, 5, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0bb8, DoBclr8, DB, SdB, 5, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0bc0, DoBset32, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0bc8, DoMovep, DL, SdL, 5, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op0bd0, DoBset8, DB, SdB, 5, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0bd8, DoBset8, DB, SdB, 5, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0be0, DoBset8, DB, SdB, 5, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0be8, DoBset8, DB, SdB, 5, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0bf0, DoBset8, DB, SdB, 5, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0bf8, DoBset8, DB, SdB, 5, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0d00, DoBtst32, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0d08, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 6, GN, SRW, 16)
Oper (Op0d10, DoBtst8, DB, SdB, 6, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0d18, DoBtst8, DB, SdB, 6, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0d20, DoBtst8, DB, SdB, 6, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0d28, DoBtst8, DB, SdB, 6, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0d30, DoBtst8, DB, SdB, 6, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0d38, DoBtst8, DB, SdB, 6, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0d40, DoBchg32, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0d48, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 6, GN, SRL, 24)
Oper (Op0d50, DoBchg8, DB, SdB, 6, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0d58, DoBchg8, DB, SdB, 6, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0d60, DoBchg8, DB, SdB, 6, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0d68, DoBchg8, DB, SdB, 6, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0d70, DoBchg8, DB, SdB, 6, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0d78, DoBchg8, DB, SdB, 6, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0d80, DoBclr32, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0d88, DoMovep, DW, SdW, 6, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0d90, DoBclr8, DB, SdB, 6, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0d98, DoBclr8, DB, SdB, 6, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0da0, DoBclr8, DB, SdB, 6, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0da8, DoBclr8, DB, SdB, 6, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0db0, DoBclr8, DB, SdB, 6, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0db8, DoBclr8, DB, SdB, 6, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0dc0, DoBset32, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0dc8, DoMovep, DL, SdL, 6, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op0dd0, DoBset8, DB, SdB, 6, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0dd8, DoBset8, DB, SdB, 6, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0de0, DoBset8, DB, SdB, 6, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0de8, DoBset8, DB, SdB, 6, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0df0, DoBset8, DB, SdB, 6, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0df8, DoBset8, DB, SdB, 6, DB, DA, Fear, ins7, GMB, SMB, 18)

OMoves (Op0e10, DB, Cain, GRB, SRB, GMB, SMB, 12)
OMoves (Op0e18, DB, CaipB, GRB, SRB, GMB, SMB, 12)
OMoves (Op0e20, DB, CmaiB, GRB, SRB, GMB, SMB, 16)
OMoves (Op0e28, DB, Cdai, GRB, SRB, GMB, SMB, 16)
OMoves (Op0e30, DB, Caix, GRB, SRB, GMB, SMB, 20)
OMoves (Op0e38, DB, Ceaw, GRB, SRB, GMB, SMB, 18)
OMoves (Op0e50, DW, Cain, GRW, SRW, GMW, SMW, 12)
OMoves (Op0e58, DW, CaipB, GRW, SRW, GMW, SMW, 12)
OMoves (Op0e60, DW, CmaiB, GRW, SRW, GMW, SMW, 16)
OMoves (Op0e68, DW, Cdai, GRW, SRW, GMW, SMW, 16)
OMoves (Op0e70, DW, Caix, GRW, SRW, GMW, SMW, 20)
OMoves (Op0e78, DW, Ceaw, GRW, SRW, GMW, SMW, 18)
OMoves (Op0e90, DL, Cain, GRL, SRL, GML, SML, 20)
OMoves (Op0e98, DL, CaipB, GRL, SRL, GML, SML, 20)
OMoves (Op0ea0, DL, CmaiB, GRL, SRL, GML, SML, 24)
OMoves (Op0ea8, DL, Cdai, GRL, SRL, GML, SML, 24)
OMoves (Op0eb0, DL, Caix, GRL, SRL, GML, SML, 28)
OMoves (Op0eb8, DL, Ceaw, GRL, SRL, GML, SML, 26)

OperS(Op0f00, DoBtst32, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SN, 8)
Oper (Op0f08, DoMovep, DW, SdaiPW, ins7, DW, DR, Cd, 7, GN, SRW, 16)
Oper (Op0f10, DoBtst8, DB, SdB, 7, DB, DA, Cain, ins7, GMB, SN, 8)
Oper (Op0f18, DoBtst8, DB, SdB, 7, DB, DA, CaipB, ins7, GMB, SN, 8)
Oper (Op0f20, DoBtst8, DB, SdB, 7, DB, DA, CmaiB, ins7, GMB, SN, 12)
Oper (Op0f28, DoBtst8, DB, SdB, 7, DB, DA, Fdai, ins7, GMB, SN, 12)
Oper (Op0f30, DoBtst8, DB, SdB, 7, DB, DA, Faix, ins7, GMB, SN, 16)
Oper (Op0f38, DoBtst8, DB, SdB, 7, DB, DA, Fear, ins7, GMB, SN, 10)
OperS(Op0f40, DoBchg32, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0f48, DoMovep, DL, SdaiPL, ins7, DL, DR, Cd, 7, GN, SRL, 24)
Oper (Op0f50, DoBchg8, DB, SdB, 7, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0f58, DoBchg8, DB, SdB, 7, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0f60, DoBchg8, DB, SdB, 7, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0f68, DoBchg8, DB, SdB, 7, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0f70, DoBchg8, DB, SdB, 7, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0f78, DoBchg8, DB, SdB, 7, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0f80, DoBclr32, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0f88, DoMovep, DW, SdW, 7, DW, DA, Fdai, ins7, GN, SPW, 16)
Oper (Op0f90, DoBclr8, DB, SdB, 7, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0f98, DoBclr8, DB, SdB, 7, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0fa0, DoBclr8, DB, SdB, 7, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0fa8, DoBclr8, DB, SdB, 7, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0fb0, DoBclr8, DB, SdB, 7, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0fb8, DoBclr8, DB, SdB, 7, DB, DA, Fear, ins7, GMB, SMB, 18)
OperS(Op0fc0, DoBset32, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 8)
Oper (Op0fc8, DoMovep, DL, SdL, 7, DL, DA, Fdai, ins7, GN, SPL, 24)
Oper (Op0fd0, DoBset8, DB, SdB, 7, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op0fd8, DoBset8, DB, SdB, 7, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op0fe0, DoBset8, DB, SdB, 7, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op0fe8, DoBset8, DB, SdB, 7, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op0ff0, DoBset8, DB, SdB, 7, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op0ff8, DoBset8, DB, SdB, 7, DB, DA, Fear, ins7, GMB, SMB, 18)

OperS(Op0800, DoBtst32, DB, SimmB, 0, DL, DR, Cd, ins7, GRL, SN, 12)
Oper (Op0810, DoBtst8, DB, SimmB, 0, DB, DA, Cain, ins7, GMB, SN, 12)
Oper (Op0818, DoBtst8, DB, SimmB, 0, DB, DA, CaipB, ins7, GMB, SN, 12)
Oper (Op0820, DoBtst8, DB, SimmB, 0, DB, DA, CmaiB, ins7, GMB, SN, 16)
Oper (Op0828, DoBtst8, DB, SimmB, 0, DB, DA, Cdai, ins7, GMB, SN, 16)
Oper (Op0830, DoBtst8, DB, SimmB, 0, DB, DA, Caix, ins7, GMB, SN, 20)
Oper (Op0838, DoBtst8, DB, SimmB, 0, DB, DA, Cear, ins7, GMB, SN, 14)
OperS(Op0840, DoBchg32, DL, SimmB, 0, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op0850, DoBchg8, DB, SimmB, 0, DB, DA, Cain, ins7, GMB, SMB, 16)
Oper (Op0858, DoBchg8, DB, SimmB, 0, DB, DA, CaipB, ins7, GMB, SMB, 16)
Oper (Op0860, DoBchg8, DB, SimmB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 20)
Oper (Op0868, DoBchg8, DB, SimmB, 0, DB, DA, Cdai, ins7, GMB, SMB, 20)
Oper (Op0870, DoBchg8, DB, SimmB, 0, DB, DA, Caix, ins7, GMB, SMB, 24)
Oper (Op0878, DoBchg8, DB, SimmB, 0, DB, DA, Cear, ins7, GMB, SMB, 22)
OperS(Op0880, DoBclr32, DL, SimmB, 0, DL, DR, Cd, ins7, GRL, SRL, 16)
Oper (Op0890, DoBclr8, DB, SimmB, 0, DB, DA, Cain, ins7, GMB, SMB, 16)
Oper (Op0898, DoBclr8, DB, SimmB, 0, DB, DA, CaipB, ins7, GMB, SMB, 16)
Oper (Op08a0, DoBclr8, DB, SimmB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 20)
Oper (Op08a8, DoBclr8, DB, SimmB, 0, DB, DA, Cdai, ins7, GMB, SMB, 20)
Oper (Op08b0, DoBclr8, DB, SimmB, 0, DB, DA, Caix, ins7, GMB, SMB, 24)
Oper (Op08b8, DoBclr8, DB, SimmB, 0, DB, DA, Cear, ins7, GMB, SMB, 22)
OperS(Op08c0, DoBset32, DL, SimmB, 0, DL, DR, Cd, ins7, GRL, SRL, 12)
Oper (Op08d0, DoBset8, DB, SimmB, 0, DB, DA, Cain, ins7, GMB, SMB, 16)
Oper (Op08d8, DoBset8, DB, SimmB, 0, DB, DA, CaipB, ins7, GMB, SMB, 16)
Oper (Op08e0, DoBset8, DB, SimmB, 0, DB, DA, CmaiB, ins7, GMB, SMB, 20)
Oper (Op08e8, DoBset8, DB, SimmB, 0, DB, DA, Cdai, ins7, GMB, SMB, 20)
Oper (Op08f0, DoBset8, DB, SimmB, 0, DB, DA, Caix, ins7, GMB, SMB, 24)
Oper (Op08f8, DoBset8, DB, SimmB, 0, DB, DA, Cear, ins7, GMB, SMB, 22)
