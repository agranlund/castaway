/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - 68k arithmetic instructions
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  12.06.2002  JH  Correct bus error/address error exception stack frame
*  13.06.2002  JH  Merged in Martin's BCD implementation (untested),
*                  completed jump table. STOP is now the only
*                  unimplemented instruction.
*  08.10.2002  JH  Fixed Z-Flag for ADD.B 0x80+0x80 and Add.W 0x8000+0x8000.
*/
#ifndef PROTOH
static char     sccsid[] = "$Id: op68karith.c,v 1.5 2002/10/08 00:18:02 jhoenig Exp $";
#define OPHANDLER
#include "68000.h"
#include "op68k.h"

/*
* Opfuncs.
*/

#if 1
#define DoCmpL(target,source) { \
    register uint32 t = (uint32)target; register uint32 s = (uint32)source; \
    __asm__ volatile ( INSTR_BEGIN "cmp.l %1,%0\n\t" INSTR_END : : "d"(t), "d"(s) : "cc" ); \
};
#define DoCmpW(target,source) { \
    register uint16 t = (uint16)target; register uint16 s = (uint16)source; \
    __asm__ volatile ( INSTR_BEGIN "cmp.w %1,%0\n\t" INSTR_END : : "d"(t), "d"(s) : "cc" ); \
};
#define DoCmpB(target,source) { \
    register uint8 t = (uint8)target; register uint8 s = (uint8)source; \
    __asm__ volatile ( INSTR_BEGIN "cmp.b %1,%0\n\t" INSTR_END : : "d"(t), "d"(s) : "cc" ); \
};
#ifndef COLDFIRE
#define DoDivs(target,source) { \
    if (source == 0) { ExceptionGroup2(DIVZ); }\
    else { __asm__ volatile ( INSTR_BEGIN "divs.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); } \
};
#define DoDivu(target,source) { \
    if (source == 0) { ExceptionGroup2(DIVZ); }\
    else { __asm__ volatile ( INSTR_BEGIN "divu.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); } \
};
#define DoMuls(target,source) { __asm__ volatile ( INSTR_BEGIN "muls.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoMulu(target,source) { __asm__ volatile ( INSTR_BEGIN "mulu.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoNBCD(target,source) { __asm__ volatile ( INSTR_BEGIN "nbcd %0\n\t"      INSTR_END : "=d"(target) : "0"(target) : "cc" ); };
#define DoABCD(target,source) { __asm__ volatile ( INSTR_BEGIN "abcd %2,%0\n\t"   INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoSBCD(target,source) { __asm__ volatile ( INSTR_BEGIN "sbcd %2,%0\n\t"   INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#endif
#endif

#ifndef DoCmpB
#define DoCmpB(target,source) {\
    register int32 tgt,cvnz=0; register int8 tgt8;\
    tgt = target - source;\
    if ((uint32) source > (uint32) target) cvnz+=Cflag;\
    tgt8=tgt;\
	if (tgt!=tgt8) cvnz+=Vflag;\
    if (tgt8==0) cvnz+=Zflag;\
	else if (tgt8<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
};
#endif

#ifndef DoCmpW
#define DoCmpW(target,source) { \
    register int32 tgt,cvnz=0; register int16 tgt16;\
    tgt = target - source;\
    if ((uint32) source > (uint32) target) cvnz+=Cflag;\
	tgt16=tgt;\
    if (tgt!=tgt16) cvnz+=Vflag;\
    if (tgt16==0) cvnz+=Zflag;\
	else if (tgt16<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
};
#endif

#ifndef DoCmpL
#define DoCmpL(target,source) { \
    /* Overflow precondition: source has different sign */\
    register int32 precon=(target>>31)-(source>>31),cvnz=0;\
    if ((uint32) source > (uint32) target) cvnz+=Cflag;\
    target -= source;\
    /* target and source now have same sign: overflow! */\
    if (precon && (target>>31)==(source>>31)) cvnz+=Vflag;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
};
#endif

#ifndef DoDivs
#define DoDivs(target,source) {\
    if (source == 0) {\
       ExceptionGroup2(DIVZ);\
    } else {\
    	int16  quot, rest;\
        if ((target / source) > 32767 || (target / source) < -32768) {\
            ForceV (1);\
            ForceC (0);\
        } else {\
            rest = target % source;\
            quot = target / source;\
            target = (rest << 16) | (uint16) quot;\
            ClrCVSetNZ ((int16) target);\
        }\
    }\
}
#endif

#ifndef DoDivu
#define DoDivu(target,source) {\
    if (source == 0) {\
        ExceptionGroup2(DIVZ);\
    } else {\
	    uint16  quot, rest;\
        if (((uint32) target / (uint16) source) & 0xffff0000) {\
            ForceV (1);\
            ForceC (0);\
        } else {\
            rest = (uint32) target % (uint16) source;\
            quot = (uint32) target / (uint16) source;\
            target = (rest << 16) | quot;\
            ClrCVSetNZ ((int16) target);\
        }\
    }\
}
#endif

#ifndef DoMuls
#define DoMuls(target,source) {\
    target = ((int16) target * source);\
    ClrCVSetNZ (target);\
}
#endif

#ifndef DoMulu
#define DoMulu(target,source) {\
    target = ((uint16) target * (uint16) source);\
    ClrCVSetNZ (target);\
}
#endif

#ifndef DoNBCD
#define DoNBCD(target, source) {\
    register uint32 lo = (target & 0xF) + GetX();\
	register uint32 hi = (target & 0xF0);\
	if (lo>10) {lo=20-lo; hi+=0x20;}\
	else if (lo>0) {lo=10-lo; hi+=0x10;}\
	if (hi>0xa0) {hi=0x140-hi; ForceX(1); ForceC(1);}\
	else if (hi>0) {hi=0xa0-hi; ForceX(1); ForceC(1);}\
	else {ForceC(0); ForceX(0);}\
	target=hi+lo;\
	if (target!=0) ForceZ(0);\
};
#endif

#ifndef DoABCD
#define DoABCD(target, source) {\
    register int32 tgt; \
    tgt = (source & 0xF) + (target & 0xF) + GetX(); \
    if(tgt >= 0xA) \
	tgt += 0x6; \
    tgt += (source & 0xF0) + (target & 0xF0); \
    if(tgt >= 0xA0) \
	tgt += 0x60; \
    SetXC (tgt & 0xFF00); \
    tgt &= 0xFF; \
    SetZ(tgt == 0); \
    target = tgt; \
};
#endif    

#ifndef DoSBCD
#define DoSBCD(target, source) {\
    register int32 tgt, hi; \
    tgt = (target & 0xF) - (source & 0xF) - GetX(); \
    hi = (target & 0xF0) - (source & 0xF0); \
    if(tgt < 0) { \
	tgt += 10; \
	hi -= 0x10; \
    } \
    if(hi < 0) { \
	hi -= 0x60; \
    } \
    tgt += hi; \
    SetXC (tgt & 0xFF00); \
    tgt &= 0xFF; \
    SetZ(tgt == 0); \
    target = tgt; \
};
#endif

#define DoCmpa(target,source) DoCmpL(target,source)
#define DoCmpm(target,source) DoCmpL(target,source)

#endif

OperS(Opc0c0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 0, GRL, SRL, 72)
OperS(Opc0d0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc0d8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc0e0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc0e8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 80)
Oper (Opc0f0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 0, GRL, SRL, 80)
OperS(Opc0f8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)

OperS(Opc1c0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 0, GRL, SRL, 72)
OperS(Opc1d0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc1d8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc1e0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)
OperS(Opc1e8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 80)
Oper (Opc1f0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 0, GRL, SRL, 80)
OperS(Opc1f8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 0, GRL, SRL, 76)

OperS(Opc2c0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 1, GRL, SRL, 72)
OperS(Opc2d0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc2d8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc2e0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc2e8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 80)
Oper (Opc2f0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 1, GRL, SRL, 80)
OperS(Opc2f8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)

OperS(Opc3c0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 1, GRL, SRL, 72)
OperS(Opc3d0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc3d8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc3e0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)
OperS(Opc3e8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 80)
Oper (Opc3f0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 1, GRL, SRL, 80)
OperS(Opc3f8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 1, GRL, SRL, 76)

OperS(Opc4c0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 2, GRL, SRL, 72)
OperS(Opc4d0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc4d8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc4e0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc4e8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 80)
Oper (Opc4f0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 2, GRL, SRL, 80)
OperS(Opc4f8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)

OperS(Opc5c0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 2, GRL, SRL, 72)
OperS(Opc5d0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc5d8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc5e0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)
OperS(Opc5e8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 80)
Oper (Opc5f0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 2, GRL, SRL, 80)
OperS(Opc5f8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 2, GRL, SRL, 76)

OperS(Opc6c0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 3, GRL, SRL, 72)
OperS(Opc6d0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc6d8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc6e0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc6e8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 80)
Oper (Opc6f0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 3, GRL, SRL, 80)
OperS(Opc6f8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)

OperS(Opc7c0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 3, GRL, SRL, 72)
OperS(Opc7d0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc7d8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc7e0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)
OperS(Opc7e8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 80)
Oper (Opc7f0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 3, GRL, SRL, 80)
OperS(Opc7f8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 3, GRL, SRL, 76)

OperS(Opc8c0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 4, GRL, SRL, 72)
OperS(Opc8d0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc8d8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc8e0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc8e8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 80)
Oper (Opc8f0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 4, GRL, SRL, 80)
OperS(Opc8f8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)

OperS(Opc9c0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 4, GRL, SRL, 72)
OperS(Opc9d0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc9d8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc9e0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)
OperS(Opc9e8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 80)
Oper (Opc9f0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 4, GRL, SRL, 80)
OperS(Opc9f8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 4, GRL, SRL, 76)

OperS(Opcac0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 5, GRL, SRL, 72)
OperS(Opcad0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcad8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcae0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcae8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 80)
Oper (Opcaf0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 5, GRL, SRL, 80)
OperS(Opcaf8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)

OperS(Opcbc0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 5, GRL, SRL, 72)
OperS(Opcbd0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcbd8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcbe0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)
OperS(Opcbe8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 80)
Oper (Opcbf0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 5, GRL, SRL, 80)
OperS(Opcbf8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 5, GRL, SRL, 76)

OperS(Opccc0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 6, GRL, SRL, 72)
OperS(Opccd0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opccd8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opcce0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opcce8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 80)
Oper (Opccf0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 6, GRL, SRL, 80)
OperS(Opccf8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)

OperS(Opcdc0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 6, GRL, SRL, 72)
OperS(Opcdd0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opcdd8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opcde0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)
OperS(Opcde8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 80)
Oper (Opcdf0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 6, GRL, SRL, 80)
OperS(Opcdf8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 6, GRL, SRL, 76)

OperS(Opcec0, DoMulu, DW, SdW, ins7, DL, DR, Cd, 7, GRL, SRL, 72)
OperS(Opced0, DoMulu, DW, SainW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opced8, DoMulu, DW, SaipW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opcee0, DoMulu, DW, SmaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opcee8, DoMulu, DW, SdaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 80)
Oper (Opcef0, DoMulu, DW, SaixW, ins7, DL, DR, Cd, 7, GRL, SRL, 80)
OperS(Opcef8, DoMulu, DW, SearW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)

OperS(Opcfc0, DoMuls, DW, SdW, ins7, DL, DR, Cd, 7, GRL, SRL, 72)
OperS(Opcfd0, DoMuls, DW, SainW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opcfd8, DoMuls, DW, SaipW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opcfe0, DoMuls, DW, SmaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)
OperS(Opcfe8, DoMuls, DW, SdaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 80)
Oper (Opcff0, DoMuls, DW, SaixW, ins7, DL, DR, Cd, 7, GRL, SRL, 80)
OperS(Opcff8, DoMuls, DW, SearW, ins7, DL, DR, Cd, 7, GRL, SRL, 76)

OperS(Opb000, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 0, GRB, SN, 4)
OperS(Opb010, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 0, GRB, SN, 8)
OperS(Opb018, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 0, GRB, SN, 8)
OperS(Opb020, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 0, GRB, SN, 12)
OperS(Opb028, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 0, GRB, SN, 12)
Oper (Opb030, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 0, GRB, SN, 16)
OperS(Opb038, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 0, GRB, SN, 10)
OperS(Opb040, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 0, GRW, SN, 4)
OperS(Opb048, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 0, GRW, SN, 4)
OperS(Opb050, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 0, GRW, SN, 8)
OperS(Opb058, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 0, GRW, SN, 8)
OperS(Opb060, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 0, GRW, SN, 12)
OperS(Opb068, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 0, GRW, SN, 12)
Oper (Opb070, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 0, GRW, SN, 16)
OperS(Opb078, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 0, GRW, SN, 10)
OperS(Opb080, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 0, GRL, SN, 8)
OperS(Opb088, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 0, GRL, SN, 8)
OperS(Opb090, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 0, GRL, SN, 16)
OperS(Opb098, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 0, GRL, SN, 16)
OperS(Opb0a0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 0, GRL, SN, 16)
OperS(Opb0a8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 0, GRL, SN, 20)
Oper (Opb0b0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 0, GRL, SN, 20)
OperS(Opb0b8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 0, GRL, SN, 16)
OperS(Opb0c0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 0, GRL, SN, 8)
OperS(Opb0c8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 0, GRL, SN, 8)
OperS(Opb0d0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 0, GRL, SN, 12)
OperS(Opb0d8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 0, GRL, SN, 12)
OperS(Opb0e0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 0, GRL, SN, 12)
OperS(Opb0e8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 0, GRL, SN, 16)
Oper (Opb0f0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 0, GRL, SN, 16)
OperS(Opb0f8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 0, GRL, SN, 12)

Oper (Opb108, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 0, GMB, SN, 12)
Oper (Opb148, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 0, GMW, SN, 12)
Oper (Opb188, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 0, GML, SN, 20)
OperS(Opb1c0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 0, GRL, SN, 8)
OperS(Opb1c8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 0, GRL, SN, 8)
OperS(Opb1d0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 0, GRL, SN, 16)
OperS(Opb1d8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 0, GRL, SN, 16)
OperS(Opb1e0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 0, GRL, SN, 16)
OperS(Opb1e8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 0, GRL, SN, 20)
Oper (Opb1f0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 0, GRL, SN, 20)
OperS(Opb1f8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 0, GRL, SN, 16)

OperS(Opb200, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 1, GRB, SN, 4)
OperS(Opb210, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 1, GRB, SN, 8)
OperS(Opb218, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 1, GRB, SN, 8)
OperS(Opb220, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 1, GRB, SN, 12)
OperS(Opb228, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 1, GRB, SN, 12)
Oper (Opb230, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 1, GRB, SN, 16)
OperS(Opb238, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 1, GRB, SN, 10)
OperS(Opb240, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 1, GRW, SN, 4)
OperS(Opb248, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 1, GRW, SN, 4)
OperS(Opb250, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 1, GRW, SN, 8)
OperS(Opb258, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 1, GRW, SN, 8)
OperS(Opb260, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 1, GRW, SN, 12)
OperS(Opb268, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 1, GRW, SN, 12)
Oper (Opb270, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 1, GRW, SN, 16)
OperS(Opb278, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 1, GRW, SN, 10)
OperS(Opb280, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 1, GRL, SN, 8)
OperS(Opb288, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 1, GRL, SN, 8)
OperS(Opb290, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 1, GRL, SN, 16)
OperS(Opb298, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 1, GRL, SN, 16)
OperS(Opb2a0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 1, GRL, SN, 16)
OperS(Opb2a8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 1, GRL, SN, 20)
Oper (Opb2b0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 1, GRL, SN, 20)
OperS(Opb2b8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 1, GRL, SN, 16)
OperS(Opb2c0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 1, GRL, SN, 8)
OperS(Opb2c8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 1, GRL, SN, 8)
OperS(Opb2d0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 1, GRL, SN, 12)
OperS(Opb2d8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 1, GRL, SN, 12)
OperS(Opb2e0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 1, GRL, SN, 12)
OperS(Opb2e8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 1, GRL, SN, 16)
Oper (Opb2f0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 1, GRL, SN, 16)
OperS(Opb2f8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 1, GRL, SN, 12)

Oper (Opb308, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 1, GMB, SN, 12)
Oper (Opb348, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 1, GMW, SN, 12)
Oper (Opb388, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 1, GML, SN, 20)
OperS(Opb3c0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 1, GRL, SN, 8)
OperS(Opb3c8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 1, GRL, SN, 8)
OperS(Opb3d0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 1, GRL, SN, 16)
OperS(Opb3d8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 1, GRL, SN, 16)
OperS(Opb3e0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 1, GRL, SN, 16)
OperS(Opb3e8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 1, GRL, SN, 20)
Oper (Opb3f0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 1, GRL, SN, 20)
OperS(Opb3f8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 1, GRL, SN, 16)

OperS(Opb400, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 2, GRB, SN, 4)
OperS(Opb410, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 2, GRB, SN, 8)
OperS(Opb418, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 2, GRB, SN, 8)
OperS(Opb420, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 2, GRB, SN, 12)
OperS(Opb428, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 2, GRB, SN, 12)
Oper (Opb430, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 2, GRB, SN, 16)
OperS(Opb438, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 2, GRB, SN, 10)
OperS(Opb440, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 2, GRW, SN, 4)
OperS(Opb448, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 2, GRW, SN, 4)
OperS(Opb450, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 2, GRW, SN, 8)
OperS(Opb458, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 2, GRW, SN, 8)
OperS(Opb460, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 2, GRW, SN, 12)
OperS(Opb468, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 2, GRW, SN, 12)
Oper (Opb470, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 2, GRW, SN, 16)
OperS(Opb478, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 2, GRW, SN, 10)
OperS(Opb480, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 2, GRL, SN, 8)
OperS(Opb488, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 2, GRL, SN, 8)
OperS(Opb490, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 2, GRL, SN, 16)
OperS(Opb498, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 2, GRL, SN, 16)
OperS(Opb4a0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 2, GRL, SN, 16)
OperS(Opb4a8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 2, GRL, SN, 20)
Oper (Opb4b0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 2, GRL, SN, 20)
OperS(Opb4b8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 2, GRL, SN, 16)
OperS(Opb4c0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 2, GRL, SN, 8)
OperS(Opb4c8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 2, GRL, SN, 8)
OperS(Opb4d0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 2, GRL, SN, 12)
OperS(Opb4d8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 2, GRL, SN, 12)
OperS(Opb4e0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 2, GRL, SN, 12)
OperS(Opb4e8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 2, GRL, SN, 16)
Oper (Opb4f0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 2, GRL, SN, 16)
OperS(Opb4f8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 2, GRL, SN, 12)

Oper (Opb508, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 2, GMB, SN, 12)
Oper (Opb548, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 2, GMW, SN, 12)
Oper (Opb588, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 2, GML, SN, 20)
OperS(Opb5c0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 2, GRL, SN, 8)
OperS(Opb5c8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 2, GRL, SN, 8)
OperS(Opb5d0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 2, GRL, SN, 16)
OperS(Opb5d8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 2, GRL, SN, 16)
OperS(Opb5e0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 2, GRL, SN, 16)
OperS(Opb5e8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 2, GRL, SN, 20)
Oper (Opb5f0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 2, GRL, SN, 20)
OperS(Opb5f8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 2, GRL, SN, 16)

OperS(Opb600, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 3, GRB, SN, 4)
OperS(Opb610, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 3, GRB, SN, 8)
OperS(Opb618, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 3, GRB, SN, 8)
OperS(Opb620, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 3, GRB, SN, 12)
OperS(Opb628, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 3, GRB, SN, 12)
Oper (Opb630, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 3, GRB, SN, 16)
OperS(Opb638, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 3, GRB, SN, 10)
OperS(Opb640, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 3, GRW, SN, 4)
OperS(Opb648, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 3, GRW, SN, 4)
OperS(Opb650, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 3, GRW, SN, 8)
OperS(Opb658, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 3, GRW, SN, 8)
OperS(Opb660, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 3, GRW, SN, 12)
OperS(Opb668, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 3, GRW, SN, 12)
Oper (Opb670, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 3, GRW, SN, 16)
OperS(Opb678, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 3, GRW, SN, 10)
OperS(Opb680, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 3, GRL, SN, 8)
OperS(Opb688, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 3, GRL, SN, 8)
OperS(Opb690, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 3, GRL, SN, 16)
OperS(Opb698, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 3, GRL, SN, 16)
OperS(Opb6a0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 3, GRL, SN, 16)
OperS(Opb6a8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 3, GRL, SN, 20)
Oper (Opb6b0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 3, GRL, SN, 20)
OperS(Opb6b8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 3, GRL, SN, 16)
OperS(Opb6c0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 3, GRL, SN, 8)
OperS(Opb6c8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 3, GRL, SN, 8)
OperS(Opb6d0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 3, GRL, SN, 12)
OperS(Opb6d8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 3, GRL, SN, 12)
OperS(Opb6e0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 3, GRL, SN, 12)
OperS(Opb6e8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 3, GRL, SN, 16)
Oper (Opb6f0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 3, GRL, SN, 16)
OperS(Opb6f8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 3, GRL, SN, 12)

Oper (Opb708, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 3, GMB, SN, 12)
Oper (Opb748, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 3, GMW, SN, 12)
Oper (Opb788, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 3, GML, SN, 20)
OperS(Opb7c0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 3, GRL, SN, 8)
OperS(Opb7c8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 3, GRL, SN, 8)
OperS(Opb7d0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 3, GRL, SN, 16)
OperS(Opb7d8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 3, GRL, SN, 16)
OperS(Opb7e0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 3, GRL, SN, 16)
OperS(Opb7e8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 3, GRL, SN, 20)
Oper (Opb7f0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 3, GRL, SN, 20)
OperS(Opb7f8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 3, GRL, SN, 16)

OperS(Opb800, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 4, GRB, SN, 4)
OperS(Opb810, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 4, GRB, SN, 8)
OperS(Opb818, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 4, GRB, SN, 8)
OperS(Opb820, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 4, GRB, SN, 12)
OperS(Opb828, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 4, GRB, SN, 12)
Oper (Opb830, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 4, GRB, SN, 16)
OperS(Opb838, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 4, GRB, SN, 10)
OperS(Opb840, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 4, GRW, SN, 4)
OperS(Opb848, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 4, GRW, SN, 4)
OperS(Opb850, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 4, GRW, SN, 8)
OperS(Opb858, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 4, GRW, SN, 8)
OperS(Opb860, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 4, GRW, SN, 12)
OperS(Opb868, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 4, GRW, SN, 12)
Oper (Opb870, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 4, GRW, SN, 16)
OperS(Opb878, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 4, GRW, SN, 10)
OperS(Opb880, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 4, GRL, SN, 8)
OperS(Opb888, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 4, GRL, SN, 8)
OperS(Opb890, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 4, GRL, SN, 16)
OperS(Opb898, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 4, GRL, SN, 16)
OperS(Opb8a0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 4, GRL, SN, 16)
OperS(Opb8a8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 4, GRL, SN, 20)
Oper (Opb8b0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 4, GRL, SN, 20)
OperS(Opb8b8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 4, GRL, SN, 16)
OperS(Opb8c0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 4, GRL, SN, 8)
OperS(Opb8c8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 4, GRL, SN, 8)
OperS(Opb8d0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 4, GRL, SN, 12)
OperS(Opb8d8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 4, GRL, SN, 12)
OperS(Opb8e0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 4, GRL, SN, 12)
OperS(Opb8e8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 4, GRL, SN, 16)
Oper (Opb8f0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 4, GRL, SN, 16)
OperS(Opb8f8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 4, GRL, SN, 12)

Oper (Opb908, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 4, GMB, SN, 12)
Oper (Opb948, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 4, GMW, SN, 12)
Oper (Opb988, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 4, GML, SN, 20)
OperS(Opb9c0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 4, GRL, SN, 8)
OperS(Opb9c8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 4, GRL, SN, 8)
OperS(Opb9d0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 4, GRL, SN, 16)
OperS(Opb9d8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 4, GRL, SN, 16)
OperS(Opb9e0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 4, GRL, SN, 16)
OperS(Opb9e8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 4, GRL, SN, 20)
Oper (Opb9f0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 4, GRL, SN, 20)
OperS(Opb9f8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 4, GRL, SN, 16)

OperS(Opba00, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 5, GRB, SN, 4)
OperS(Opba10, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 5, GRB, SN, 8)
OperS(Opba18, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 5, GRB, SN, 8)
OperS(Opba20, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 5, GRB, SN, 12)
OperS(Opba28, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 5, GRB, SN, 12)
Oper (Opba30, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 5, GRB, SN, 16)
OperS(Opba38, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 5, GRB, SN, 10)
OperS(Opba40, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 5, GRW, SN, 4)
OperS(Opba48, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 5, GRW, SN, 4)
OperS(Opba50, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 5, GRW, SN, 8)
OperS(Opba58, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 5, GRW, SN, 8)
OperS(Opba60, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 5, GRW, SN, 12)
OperS(Opba68, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 5, GRW, SN, 12)
Oper (Opba70, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 5, GRW, SN, 16)
OperS(Opba78, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 5, GRW, SN, 10)
OperS(Opba80, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 5, GRL, SN, 8)
OperS(Opba88, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 5, GRL, SN, 8)
OperS(Opba90, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 5, GRL, SN, 16)
OperS(Opba98, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 5, GRL, SN, 16)
OperS(Opbaa0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 5, GRL, SN, 16)
OperS(Opbaa8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 5, GRL, SN, 20)
Oper (Opbab0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 5, GRL, SN, 20)
OperS(Opbab8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 5, GRL, SN, 16)
OperS(Opbac0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 5, GRL, SN, 8)
OperS(Opbac8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 5, GRL, SN, 8)
OperS(Opbad0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 5, GRL, SN, 12)
OperS(Opbad8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 5, GRL, SN, 12)
OperS(Opbae0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 5, GRL, SN, 12)
OperS(Opbae8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 5, GRL, SN, 16)
Oper (Opbaf0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 5, GRL, SN, 16)
OperS(Opbaf8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 5, GRL, SN, 12)

Oper (Opbb08, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 5, GMB, SN, 12)
Oper (Opbb48, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 5, GMW, SN, 12)
Oper (Opbb88, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 5, GML, SN, 20)
OperS(Opbbc0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 5, GRL, SN, 8)
OperS(Opbbc8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 5, GRL, SN, 8)
OperS(Opbbd0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 5, GRL, SN, 16)
OperS(Opbbd8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 5, GRL, SN, 16)
OperS(Opbbe0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 5, GRL, SN, 16)
OperS(Opbbe8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 5, GRL, SN, 20)
Oper (Opbbf0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 5, GRL, SN, 20)
OperS(Opbbf8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 5, GRL, SN, 16)

OperS(Opbc00, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 6, GRB, SN, 4)
OperS(Opbc10, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 6, GRB, SN, 8)
OperS(Opbc18, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 6, GRB, SN, 8)
OperS(Opbc20, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 6, GRB, SN, 12)
OperS(Opbc28, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 6, GRB, SN, 12)
Oper (Opbc30, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 6, GRB, SN, 16)
OperS(Opbc38, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 6, GRB, SN, 10)
OperS(Opbc40, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 6, GRW, SN, 4)
OperS(Opbc48, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 6, GRW, SN, 4)
OperS(Opbc50, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 6, GRW, SN, 8)
OperS(Opbc58, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 6, GRW, SN, 8)
OperS(Opbc60, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 6, GRW, SN, 12)
OperS(Opbc68, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 6, GRW, SN, 12)
Oper (Opbc70, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 6, GRW, SN, 16)
OperS(Opbc78, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 6, GRW, SN, 10)
OperS(Opbc80, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 6, GRL, SN, 8)
OperS(Opbc88, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 6, GRL, SN, 8)
OperS(Opbc90, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 6, GRL, SN, 16)
OperS(Opbc98, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 6, GRL, SN, 16)
OperS(Opbca0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 6, GRL, SN, 16)
OperS(Opbca8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 6, GRL, SN, 20)
Oper (Opbcb0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 6, GRL, SN, 20)
OperS(Opbcb8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 6, GRL, SN, 16)
OperS(Opbcc0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 6, GRL, SN, 8)
OperS(Opbcc8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 6, GRL, SN, 8)
OperS(Opbcd0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 6, GRL, SN, 12)
OperS(Opbcd8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 6, GRL, SN, 12)
OperS(Opbce0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 6, GRL, SN, 12)
OperS(Opbce8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 6, GRL, SN, 16)
Oper (Opbcf0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 6, GRL, SN, 16)
OperS(Opbcf8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 6, GRL, SN, 12)

Oper (Opbd08, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB, 6, GMB, SN, 12)
Oper (Opbd48, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 6, GMW, SN, 12)
Oper (Opbd88, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 6, GML, SN, 20)
OperS(Opbdc0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 6, GRL, SN, 8)
OperS(Opbdc8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 6, GRL, SN, 8)
OperS(Opbdd0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 6, GRL, SN, 16)
OperS(Opbdd8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 6, GRL, SN, 16)
OperS(Opbde0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 6, GRL, SN, 16)
OperS(Opbde8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 6, GRL, SN, 20)
Oper (Opbdf0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 6, GRL, SN, 20)
OperS(Opbdf8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 6, GRL, SN, 16)

OperS(Opbe00, DoCmpB, DB, SdB, ins7, DB, DR, Cd, 7, GRB, SN, 4)
OperS(Opbe10, DoCmpB, DB, SainB, ins7, DB, DR, Cd, 7, GRB, SN, 8)
OperS(Opbe18, DoCmpB, DB, SaipB, ins7, DB, DR, Cd, 7, GRB, SN, 8)
OperS(Opbe20, DoCmpB, DB, SmaiB, ins7, DB, DR, Cd, 7, GRB, SN, 12)
OperS(Opbe28, DoCmpB, DB, SdaiB, ins7, DB, DR, Cd, 7, GRB, SN, 12)
Oper (Opbe30, DoCmpB, DB, SaixB, ins7, DB, DR, Cd, 7, GRB, SN, 16)
OperS(Opbe38, DoCmpB, DB, SearB, ins7, DB, DR, Cd, 7, GRB, SN, 10)
OperS(Opbe40, DoCmpW, DW, SdW, ins7, DW, DR, Cd, 7, GRW, SN, 4)
OperS(Opbe48, DoCmpW, DW, SaW, ins7, DW, DR, Cd, 7, GRW, SN, 4)
OperS(Opbe50, DoCmpW, DW, SainW, ins7, DW, DR, Cd, 7, GRW, SN, 8)
OperS(Opbe58, DoCmpW, DW, SaipW, ins7, DW, DR, Cd, 7, GRW, SN, 8)
OperS(Opbe60, DoCmpW, DW, SmaiW, ins7, DW, DR, Cd, 7, GRW, SN, 12)
OperS(Opbe68, DoCmpW, DW, SdaiW, ins7, DW, DR, Cd, 7, GRW, SN, 12)
Oper (Opbe70, DoCmpW, DW, SaixW, ins7, DW, DR, Cd, 7, GRW, SN, 16)
OperS(Opbe78, DoCmpW, DW, SearW, ins7, DW, DR, Cd, 7, GRW, SN, 10)
OperS(Opbe80, DoCmpL, DL, SdL, ins7, DL, DR, Cd, 7, GRL, SN, 8)
OperS(Opbe88, DoCmpL, DL, SaL, ins7, DL, DR, Cd, 7, GRL, SN, 8)
OperS(Opbe90, DoCmpL, DL, SainL, ins7, DL, DR, Cd, 7, GRL, SN, 16)
OperS(Opbe98, DoCmpL, DL, SaipL, ins7, DL, DR, Cd, 7, GRL, SN, 16)
OperS(Opbea0, DoCmpL, DL, SmaiL, ins7, DL, DR, Cd, 7, GRL, SN, 16)
OperS(Opbea8, DoCmpL, DL, SdaiL, ins7, DL, DR, Cd, 7, GRL, SN, 20)
Oper (Opbeb0, DoCmpL, DL, SaixL, ins7, DL, DR, Cd, 7, GRL, SN, 20)
OperS(Opbeb8, DoCmpL, DL, SearL, ins7, DL, DR, Cd, 7, GRL, SN, 16)
OperS(Opbec0, DoCmpa, DL, SdW, ins7, DL, DR, Ca, 7, GRL, SN, 8)
OperS(Opbec8, DoCmpa, DL, SaW, ins7, DL, DR, Ca, 7, GRL, SN, 8)
OperS(Opbed0, DoCmpa, DL, SainW, ins7, DL, DR, Ca, 7, GRL, SN, 12)
OperS(Opbed8, DoCmpa, DL, SaipW, ins7, DL, DR, Ca, 7, GRL, SN, 12)
OperS(Opbee0, DoCmpa, DL, SmaiW, ins7, DL, DR, Ca, 7, GRL, SN, 12)
OperS(Opbee8, DoCmpa, DL, SdaiW, ins7, DL, DR, Ca, 7, GRL, SN, 16)
Oper (Opbef0, DoCmpa, DL, SaixW, ins7, DL, DR, Ca, 7, GRL, SN, 16)
OperS(Opbef8, DoCmpa, DL, SearW, ins7, DL, DR, Ca, 7, GRL, SN, 12)

Oper (Opbf08, DoCmpm, DB, SaipB, ins7, DB, DA, CaipB15, 7, GMB, SN, 12)
Oper (Opbf48, DoCmpm, DW, SaipW, ins7, DW, DA, CaipW, 7, GMW, SN, 12)
Oper (Opbf88, DoCmpm, DL, SaipL, ins7, DL, DA, CaipL, 7, GML, SN, 20)
OperS(Opbfc0, DoCmpa, DL, SdL, ins7, DL, DR, Ca, 7, GRL, SN, 8)
OperS(Opbfc8, DoCmpa, DL, SaL, ins7, DL, DR, Ca, 7, GRL, SN, 8)
OperS(Opbfd0, DoCmpa, DL, SainL, ins7, DL, DR, Ca, 7, GRL, SN, 16)
OperS(Opbfd8, DoCmpa, DL, SaipL, ins7, DL, DR, Ca, 7, GRL, SN, 16)
OperS(Opbfe0, DoCmpa, DL, SmaiL, ins7, DL, DR, Ca, 7, GRL, SN, 16)
OperS(Opbfe8, DoCmpa, DL, SdaiL, ins7, DL, DR, Ca, 7, GRL, SN, 20)
Oper (Opbff0, DoCmpa, DL, SaixL, ins7, DL, DR, Ca, 7, GRL, SN, 20)
OperS(Opbff8, DoCmpa, DL, SearL, ins7, DL, DR, Ca, 7, GRL, SN, 16)

Oper (Op80c0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 0, GRL, SRL, 140)
Oper (Op80d0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 0, GRL, SRL, 144)
Oper (Op80d8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 0, GRL, SRL, 144)
Oper (Op80e0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 148)
Oper (Op80e8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 148)
Oper (Op80f0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 0, GRL, SRL, 152)
Oper (Op80f8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 0, GRL, SRL, 146)

Oper (Op81c0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 0, GRL, SRL, 160)
Oper (Op81d0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 0, GRL, SRL, 164)
Oper (Op81d8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 0, GRL, SRL, 164)
Oper (Op81e0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 164)
Oper (Op81e8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 0, GRL, SRL, 168)
Oper (Op81f0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 0, GRL, SRL, 168)
Oper (Op81f8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 0, GRL, SRL, 164)

Oper (Op82c0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 1, GRL, SRL, 140)
Oper (Op82d0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 1, GRL, SRL, 144)
Oper (Op82d8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 1, GRL, SRL, 144)
Oper (Op82e0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 148)
Oper (Op82e8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 148)
Oper (Op82f0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 1, GRL, SRL, 152)
Oper (Op82f8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 1, GRL, SRL, 146)

Oper (Op83c0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 1, GRL, SRL, 160)
Oper (Op83d0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 1, GRL, SRL, 164)
Oper (Op83d8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 1, GRL, SRL, 164)
Oper (Op83e0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 164)
Oper (Op83e8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 1, GRL, SRL, 168)
Oper (Op83f0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 1, GRL, SRL, 168)
Oper (Op83f8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 1, GRL, SRL, 164)

Oper (Op84c0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 2, GRL, SRL, 140)
Oper (Op84d0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 2, GRL, SRL, 144)
Oper (Op84d8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 2, GRL, SRL, 144)
Oper (Op84e0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 148)
Oper (Op84e8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 148)
Oper (Op84f0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 2, GRL, SRL, 152)
Oper (Op84f8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 2, GRL, SRL, 146)

Oper (Op85c0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 2, GRL, SRL, 160)
Oper (Op85d0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 2, GRL, SRL, 164)
Oper (Op85d8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 2, GRL, SRL, 164)
Oper (Op85e0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 164)
Oper (Op85e8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 2, GRL, SRL, 168)
Oper (Op85f0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 2, GRL, SRL, 168)
Oper (Op85f8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 2, GRL, SRL, 164)

Oper (Op86c0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 3, GRL, SRL, 140)
Oper (Op86d0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 3, GRL, SRL, 144)
Oper (Op86d8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 3, GRL, SRL, 144)
Oper (Op86e0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 148)
Oper (Op86e8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 148)
Oper (Op86f0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 3, GRL, SRL, 152)
Oper (Op86f8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 3, GRL, SRL, 146)

Oper (Op87c0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 3, GRL, SRL, 160)
Oper (Op87d0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 3, GRL, SRL, 164)
Oper (Op87d8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 3, GRL, SRL, 164)
Oper (Op87e0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 164)
Oper (Op87e8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 3, GRL, SRL, 168)
Oper (Op87f0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 3, GRL, SRL, 168)
Oper (Op87f8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 3, GRL, SRL, 164)

Oper (Op88c0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 4, GRL, SRL, 140)
Oper (Op88d0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 4, GRL, SRL, 144)
Oper (Op88d8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 4, GRL, SRL, 144)
Oper (Op88e0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 148)
Oper (Op88e8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 148)
Oper (Op88f0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 4, GRL, SRL, 152)
Oper (Op88f8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 4, GRL, SRL, 146)

Oper (Op89c0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 4, GRL, SRL, 160)
Oper (Op89d0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 4, GRL, SRL, 164)
Oper (Op89d8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 4, GRL, SRL, 164)
Oper (Op89e0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 164)
Oper (Op89e8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 4, GRL, SRL, 168)
Oper (Op89f0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 4, GRL, SRL, 168)
Oper (Op89f8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 4, GRL, SRL, 164)

Oper (Op8ac0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 5, GRL, SRL, 140)
Oper (Op8ad0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 5, GRL, SRL, 144)
Oper (Op8ad8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 5, GRL, SRL, 144)
Oper (Op8ae0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 148)
Oper (Op8ae8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 148)
Oper (Op8af0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 5, GRL, SRL, 152)
Oper (Op8af8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 5, GRL, SRL, 146)

Oper (Op8bc0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 5, GRL, SRL, 160)
Oper (Op8bd0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 5, GRL, SRL, 164)
Oper (Op8bd8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 5, GRL, SRL, 164)
Oper (Op8be0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 164)
Oper (Op8be8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 5, GRL, SRL, 168)
Oper (Op8bf0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 5, GRL, SRL, 168)
Oper (Op8bf8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 5, GRL, SRL, 164)

Oper (Op8cc0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 6, GRL, SRL, 140)
Oper (Op8cd0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 6, GRL, SRL, 144)
Oper (Op8cd8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 6, GRL, SRL, 144)
Oper (Op8ce0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 148)
Oper (Op8ce8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 148)
Oper (Op8cf0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 6, GRL, SRL, 152)
Oper (Op8cf8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 6, GRL, SRL, 146)

Oper (Op8dc0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 6, GRL, SRL, 160)
Oper (Op8dd0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 6, GRL, SRL, 164)
Oper (Op8dd8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 6, GRL, SRL, 164)
Oper (Op8de0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 164)
Oper (Op8de8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 6, GRL, SRL, 168)
Oper (Op8df0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 6, GRL, SRL, 168)
Oper (Op8df8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 6, GRL, SRL, 164)

Oper (Op8ec0, DoDivu, DW, SdW, ins7, DL, DR, Cd, 7, GRL, SRL, 140)
Oper (Op8ed0, DoDivu, DW, SainW, ins7, DL, DR, Cd, 7, GRL, SRL, 144)
Oper (Op8ed8, DoDivu, DW, SaipW, ins7, DL, DR, Cd, 7, GRL, SRL, 144)
Oper (Op8ee0, DoDivu, DW, SmaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 148)
Oper (Op8ee8, DoDivu, DW, SdaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 148)
Oper (Op8ef0, DoDivu, DW, SaixW, ins7, DL, DR, Cd, 7, GRL, SRL, 152)
Oper (Op8ef8, DoDivu, DW, SearW, ins7, DL, DR, Cd, 7, GRL, SRL, 146)

Oper (Op8fc0, DoDivs, DW, SdW, ins7, DL, DR, Cd, 7, GRL, SRL, 160)
Oper (Op8fd0, DoDivs, DW, SainW, ins7, DL, DR, Cd, 7, GRL, SRL, 164)
Oper (Op8fd8, DoDivs, DW, SaipW, ins7, DL, DR, Cd, 7, GRL, SRL, 164)
Oper (Op8fe0, DoDivs, DW, SmaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 164)
Oper (Op8fe8, DoDivs, DW, SdaiW, ins7, DL, DR, Cd, 7, GRL, SRL, 168)
Oper (Op8ff0, DoDivs, DW, SaixW, ins7, DL, DR, Cd, 7, GRL, SRL, 168)
Oper (Op8ff8, DoDivs, DW, SearW, ins7, DL, DR, Cd, 7, GRL, SRL, 164)

OperS(Op0c00, DoCmpB, DB, SimmB, 0, DB, DR, Cd, ins7, GRB, SN, 8)
Oper (Op0c10, DoCmpB, DB, SimmB, 0, DB, DA, Cain, ins7, GMB, SN, 12)
Oper (Op0c18, DoCmpB, DB, SimmB, 0, DB, DA, CaipB, ins7, GMB, SN, 12)
Oper (Op0c20, DoCmpB, DB, SimmB, 0, DB, DA, CmaiB, ins7, GMB, SN, 16)
Oper (Op0c28, DoCmpB, DB, SimmB, 0, DB, DA, Cdai, ins7, GMB, SN, 16)
Oper (Op0c30, DoCmpB, DB, SimmB, 0, DB, DA, Caix, ins7, GMB, SN, 20)
Oper (Op0c38, DoCmpB, DB, SimmB, 0, DB, DA, Ceaw, ins7, GMB, SN, 18)
OperS(Op0c40, DoCmpW, DW, SimmW, 0, DW, DR, Cd, ins7, GRW, SN, 8)
Oper (Op0c50, DoCmpW, DW, SimmW, 0, DW, DA, Cain, ins7, GMW, SN, 12)
Oper (Op0c58, DoCmpW, DW, SimmW, 0, DW, DA, CaipW, ins7, GMW, SN, 12)
Oper (Op0c60, DoCmpW, DW, SimmW, 0, DW, DA, CmaiW, ins7, GMW, SN, 16)
Oper (Op0c68, DoCmpW, DW, SimmW, 0, DW, DA, Cdai, ins7, GMW, SN, 16)
Oper (Op0c70, DoCmpW, DW, SimmW, 0, DW, DA, Caix, ins7, GMW, SN, 20)
Oper (Op0c78, DoCmpW, DW, SimmW, 0, DW, DA, Ceaw, ins7, GMW, SN, 18)
OperS(Op0c80, DoCmpL, DL, SimmL, 0, DL, DR, Cd, ins7, GRL, SN, 16)
Oper (Op0c90, DoCmpL, DL, SimmL, 0, DL, DA, Cain, ins7, GML, SN, 20)
Oper (Op0c98, DoCmpL, DL, SimmL, 0, DL, DA, CaipL, ins7, GML, SN, 20)
Oper (Op0ca0, DoCmpL, DL, SimmL, 0, DL, DA, CmaiL, ins7, GML, SN, 24)
Oper (Op0ca8, DoCmpL, DL, SimmL, 0, DL, DA, Cdai, ins7, GML, SN, 24)
Oper (Op0cb0, DoCmpL, DL, SimmL, 0, DL, DA, Caix, ins7, GML, SN, 28)
Oper (Op0cb8, DoCmpL, DL, SimmL, 0, DL, DA, Ceaw, ins7, GML, SN, 26)

Oper (Opc100, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 0, GRB, SRB, 8)
Oper (Opc108, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 0, GMB, SMB, 20)
Oper (Opc300, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 1, GRB, SRB, 8)
Oper (Opc308, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 1, GMB, SMB, 20)
Oper (Opc500, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 2, GRB, SRB, 8)
Oper (Opc508, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 2, GMB, SMB, 20)
Oper (Opc700, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 3, GRB, SRB, 8)
Oper (Opc708, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 3, GMB, SMB, 20)
Oper (Opc900, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 4, GRB, SRB, 8)
Oper (Opc908, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 4, GMB, SMB, 20)
Oper (Opcb00, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 5, GRB, SRB, 8)
Oper (Opcb08, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 5, GMB, SMB, 20)
Oper (Opcd00, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 6, GRB, SRB, 8)
Oper (Opcd08, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 6, GMB, SMB, 20)
Oper (Opcf00, DoABCD, DB, SdB,   ins7, DB, DR, Cd, 7, GRB, SRB, 8)
Oper (Opcf08, DoABCD, DB, SmaiB, ins7, DB, DA, CmaiB, 7, GMB, SMB, 20)

Oper (Op8100, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 0, GRB, SRB, 8)
Oper (Op8108, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 0, GMB, SMB, 20)
Oper (Op8300, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 1, GRB, SRB, 8)
Oper (Op8308, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 1, GMB, SMB, 20)
Oper (Op8500, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 2, GRB, SRB, 8)
Oper (Op8508, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 2, GMB, SMB, 20)
Oper (Op8700, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 3, GRB, SRB, 8)
Oper (Op8708, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 3, GMB, SMB, 20)
Oper (Op8900, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 4, GRB, SRB, 8)
Oper (Op8908, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 4, GMB, SMB, 20)
Oper (Op8b00, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 5, GRB, SRB, 8)
Oper (Op8b08, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 5, GMB, SMB, 20)
Oper (Op8d00, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 6, GRB, SRB, 8)
Oper (Op8d08, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 6, GMB, SMB, 20)
Oper (Op8f00, DoSBCD, DB, SdB,   ins7, DB, DR, Cd, 7, GRB, SRB, 8)
Oper (Op8f08, DoSBCD, DB, SmaiB, ins7, DB, DA, CmaiB, 7, GMB, SMB, 20)

Oper (Op4800, DoNBCD, DN, SNN, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
Oper (Op4810, DoNBCD, DN, SNN, 0, DB, DA, Cain, ins7, GMB, SMB, 12)
Oper (Op4818, DoNBCD, DN, SNN, 0, DB, DA, CaipB, ins7, GMB, SMB, 12)
Oper (Op4820, DoNBCD, DN, SNN, 0, DB, DA, CmaiB, ins7, GMB, SMB, 16)
Oper (Op4828, DoNBCD, DN, SNN, 0, DB, DA, Fdai, ins7, GMB, SMB, 16)
Oper (Op4830, DoNBCD, DN, SNN, 0, DB, DA, Faix, ins7, GMB, SMB, 20)
Oper (Op4838, DoNBCD, DN, SNN, 0, DB, DA, Feaw, ins7, GMB, SMB, 18)