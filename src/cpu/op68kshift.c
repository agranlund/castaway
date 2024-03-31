/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - 68k shift instructions
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  22.06.2002  JH  V-flag after ASL was broken in some special cases.
*  09.07.2002  JH  ROXL/ROXR: X-flag not modified when shift count is 0.
*  10.07.2002  JH  other shift ops: X-flag not modified when shift count is 0.
*  10.09.2002  JH  Bugfix: ROXL.L
*  15.09.2002  JH  Minor SR/Bcc/Scc optimization. Fixed V-Flag for ASL.L.
*                  Wrapped 68k operand types in typedefs.
*  01.10.2002  JH  Bugfix: -(Ax) and (Ax)+ addressing modes were swapped.
*                  *** Many thx to olivencia@wanado.fr for finding this!! ***
*  02.10.2002  JH  SetV simplified for ASL.
*/
#ifndef PROTOH
static char     sccsid[] = "$Id: op68kshift.c,v 1.10 2002/10/05 08:25:08 jhoenig Exp $";
#define OPHANDLER
#include "68000.h"
#include "op68k.h"

#define addcycles ADD_CYCLES(source<<1)

#if 1
#define DoLslL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsl.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoLsrL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsr.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#ifndef COLDFIRE
#define DoLslB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsl.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoLslW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsl.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoLsrB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsr.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoLsrW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "lsr.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAslB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asl.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAslW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asl.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAslL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asl.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAsrB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asr.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAsrW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asr.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoAsrL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "asr.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); };
#define DoRoxlB(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxl.b %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRoxlW(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxl.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRoxlL(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxl.l %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRoxrB(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxr.b %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRoxrW(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxr.w %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRoxrL(target,source) { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "roxr.l %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRolB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "rol.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRolW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "rol.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRolL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "rol.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRorB(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "ror.b  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRorW(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "ror.w  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#define DoRorL(target,source)  { source&=0x3f; addcycles; __asm__ volatile ( INSTR_BEGIN "ror.l  %2,%0\n\t" INSTR_END : "=d"(target) : "0"(target), "d"(source) : "cc" ); }
#endif
#endif

#ifndef DoAslB
#define DoAslB(target,source) \
	source&=0x3f; \
	addcycles \
    if (source> 0) {\
	register uint32 cvnz=0;\
	register uint32 tgt=target<<source;\
	if (tgt&0x100) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	if ((tgt&0x80)!=(target&0x80)) cvnz+=Vflag;\
	target=tgt;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
}
#endif
#ifndef DoAslW
#define DoAslW(target,source) \
	source&=0x3f; \
	addcycles \
    if (source> 0) {\
	register uint32 cvnz=0;\
	register uint32 tgt=target<<source;\
	if (tgt&0x10000) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	if ((tgt&0x8000)!=(target&0x8000)) cvnz+=Vflag;\
	target=tgt;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
}
#endif
#ifndef DoAslL
#define DoAslL(target,source) \
	source&=0x3f; \
	addcycles \
    if (source>0) {\
	register uint32 cvnz=0;\
	register uint32 tgt=target<<(source-1);\
	if (tgt&0x80000000) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	tgt<<=1;\
	if ((((uint32)target)>>31)!=(tgt>>31)) cvnz+=Vflag;\
	target=tgt;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
}
#endif
#if !defined(DoAsrB) || !defined(DoAsrW) || !defined(DoAsrL)
#define DoAsr(target,source) \
	source&=0x3f; \
	addcycles \
    if (source>0) {\
	register uint32 cvnz=0;\
	target >>= source - 1;\
	if (target & 0x1) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	target >>= 1;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
}
#ifndef DoAsrB
#define DoAsrB(target,source) DoAsr(target,source)
#endif
#ifndef DoAsrW
#define DoAsrW(target,source) DoAsr(target,source)
#endif
#ifndef DoAsrL
#define DoAsrL(target,source) DoAsr(target,source)
#endif
#endif

#ifndef DoLslB
#define DoLslB(target,source) \
	source&=0x3f; \
	addcycles \
    if (source>0) {\
	register uint32 cvnz=0;\
	register uint32 tgt=target<<source;\
	if (target&0x100) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	target=tgt;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
}
#endif
#ifndef DoLslW
#define DoLslW(target,source) \
	source&=0x3f; \
	addcycles \
    if (source>0) {\
	register uint32 cvnz=0;\
	register uint32 tgt=target<<source;\
	if (target&0x10000) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	target=tgt;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
    }
#endif
#ifndef DoLslL
#define DoLslL(target,source) \
	source&=0x3f; \
	addcycles \
    if (source> 0) {\
	register uint32 cvnz=0;\
	target <<= source - 1;\
	if (target & 0x80000000) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	target <<= 1;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
    }
#endif
#if !defined(DoLsrB) || !defined(DoLsrW) || !defined(DoLsrL)
#define DoLsr(target,source,type) \
	source&=0x3f; \
	addcycles \
    if (source> 0) {\
	register uint32 cvnz=0;\
	if (target&(1<<(source-1))) {cvnz+=Cflag; ForceX(1);} else ForceX(0);\
	target=((type)target)>>source;\
	if (target==0) cvnz+=Zflag;\
	else if (target<0) cvnz+=Nflag;\
	ForceCVNZ(cvnz);\
    } else {\
	ClrCVSetNZ(target);\
    }
#ifndef DoLsrB
#define DoLsrB(target,source) DoLsr(target,source,uint8)
#endif
#ifndef DoLsrW
#define DoLsrW(target,source) DoLsr(target,source,uint16)
#endif
#ifndef DoLsrL
#define DoLsrL(target,source) DoLsr(target,source,uint32)
#endif
#endif

#ifndef DoRoxlB
#define DoRoxlB(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint16 tgt1, tgt2;\
	source %= 9;\
	tgt1 = (uint8) target;\
	tgt1 |= (uint16) (GetX()) << 8;\
	tgt2 = tgt1;\
	tgt2 >>= (9 - source);\
	tgt1 <<= source;\
	tgt1 |= tgt2;\
	SetXC ((tgt1 >> 8) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRoxlW
#define DoRoxlW(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint32 tgt1, tgt2;\
	source %= 17;\
	tgt1 = (uint16) target;\
	tgt1 |= (uint32) (GetX()) << 16;\
	tgt2 = tgt1;\
	tgt2 >>= (17 - source);\
	tgt1 <<= source;\
	tgt1 |= tgt2;\
	SetXC ((tgt1 >> 16) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRoxlL
#define DoRoxlL(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint32 tgt1, tgt2;\
	source %= 33;\
	tgt2 = tgt1 = (uint32) target;\
	if (source != 0) {\
	tgt1 <<= 1;\
	tgt1 |= (uint32) (GetX());\
	tgt1 <<= (source - 1);\
	tgt2 >>= (32 - source);\
	SetXC (tgt2 & 0x1);\
	tgt2 >>= 1;\
	tgt1 |= tgt2;\
	target = tgt1;\
	}\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRoxrB
#define DoRoxrB(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint16 tgt1, tgt2;\
	source %= 9;\
	tgt1 = (uint8) target;\
	tgt1 |= (uint16) (GetX()) << 8;\
	tgt2 = tgt1;\
	tgt2 <<= (9 - source);\
	tgt1 >>= source;\
	tgt1 |= tgt2;\
	SetXC ((tgt1 >> 8) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRoxrW
#define DoRoxrW(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint32 tgt1, tgt2;\
	source %= 17;\
	tgt1 = (uint16) target;\
	tgt1 |= (uint32) (GetX()) << 16;\
	tgt2 = tgt1;\
	tgt2 <<= (17 - source);\
	tgt1 >>= source;\
	tgt1 |= tgt2;\
	SetXC ((tgt1 >> 16) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRoxrL
#define DoRoxrL(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	SetC(GetX());\
    } else {\
	register uint32 tgt1, tgt2;\
	source %= 33;\
	tgt2 = tgt1 = (uint32) target;\
	if (source != 0) {\
	tgt2 <<= 1;\
	tgt2 |= (uint32) (GetX());\
	tgt2 <<= (32 - source);\
	tgt1 >>= (source - 1);\
	SetXC (tgt1 & 0x1);\
	tgt1 >>= 1;\
	tgt1 |= tgt2;\
	target = tgt1;\
	}\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRolB
#define DoRolB(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint8 tgt1, tgt2;\
	source &= 0x7;\
	tgt2 = tgt1 = target;\
	tgt2 >>= (8 - source);\
	tgt1 <<= source;\
	tgt1 |= tgt2;\
	SetC (tgt1 & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRolW
#define DoRolW(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint16 tgt1, tgt2;\
	source &= 0xf;\
	tgt2 = tgt1 = target;\
	tgt2 >>= (16 - source);\
	tgt1 <<= source;\
	tgt1 |= tgt2;\
	SetC (tgt1 & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRolL
#define DoRolL(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint32 tgt1, tgt2;\
	source &= 0x1f;\
	tgt2 = tgt1 = (uint32) target;\
	tgt2 >>= (32 - source);\
	tgt1 <<= source;\
	tgt1 |= tgt2;\
	SetC (tgt1 & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRorB
#define DoRorB(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint8 tgt1, tgt2;\
	source &= 0x7;\
	tgt2 = tgt1 = target;\
	tgt2 <<= (8 - source);\
	tgt1 >>= source;\
	tgt1 |= tgt2;\
	SetC ((tgt1 >> 7) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRorW
#define DoRorW(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint16 tgt1, tgt2;\
	source &= 0xf;\
	tgt2 = tgt1 = target;\
	tgt2 <<= (16 - source);\
	tgt1 >>= source;\
	tgt1 |= tgt2;\
	SetC ((tgt1 >> 15) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif
#ifndef DoRorL
#define DoRorL(target,source) {\
    source &= 0x3f;\
	addcycles \
    ForceV (0);\
    if (source == 0) {\
	ForceC (0);\
    } else {\
	register uint32 tgt1, tgt2;\
	source &= 0x1f;\
	tgt2 = tgt1 = target;\
	tgt2 <<= (32 - source);\
	tgt1 >>= source;\
	tgt1 |= tgt2;\
	SetC ((tgt1 >> 31) & 0x1);\
	target = tgt1;\
    }\
    SetNZ (target);\
}
#endif

#endif

OperS(Ope000, DoAsrB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope008, DoLsrB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope010, DoRoxrB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope018, DoRorB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope020, DoAsrB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope028, DoLsrB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope030, DoRoxrB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope038, DoRorB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope040, DoAsrW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope048, DoLsrW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope050, DoRoxrW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope058, DoRorW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope060, DoAsrW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope068, DoLsrW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope070, DoRoxrW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope078, DoRorW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope080, DoAsrL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope088, DoLsrL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope090, DoRoxrL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope098, DoRorL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope0a0, DoAsrL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope0a8, DoLsrL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope0b0, DoRoxrL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope0b8, DoRorL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope100, DoAslB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope108, DoLslB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope110, DoRoxlB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope118, DoRolB, DB, Ss, 8, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope120, DoAslB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope128, DoLslB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope130, DoRoxlB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope138, DoRolB, DB, SdB, 0, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope140, DoAslW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope148, DoLslW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope150, DoRoxlW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope158, DoRolW, DW, Ss, 8, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope160, DoAslW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope168, DoLslW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope170, DoRoxlW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope178, DoRolW, DW, SdW, 0, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope180, DoAslL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope188, DoLslL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope190, DoRoxlL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope198, DoRolL, DL, Ss, 8, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope1a0, DoAslL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope1a8, DoLslL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope1b0, DoRoxlL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope1b8, DoRolL, DL, SdL, 0, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope200, DoAsrB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope208, DoLsrB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope210, DoRoxrB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope218, DoRorB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope220, DoAsrB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope228, DoLsrB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope230, DoRoxrB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope238, DoRorB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope240, DoAsrW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope248, DoLsrW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope250, DoRoxrW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope258, DoRorW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope260, DoAsrW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope268, DoLsrW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope270, DoRoxrW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope278, DoRorW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope280, DoAsrL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope288, DoLsrL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope290, DoRoxrL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope298, DoRorL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope2a0, DoAsrL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope2a8, DoLsrL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope2b0, DoRoxrL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope2b8, DoRorL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope300, DoAslB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope308, DoLslB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope310, DoRoxlB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope318, DoRolB, DB, Ss, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope320, DoAslB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope328, DoLslB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope330, DoRoxlB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope338, DoRolB, DB, SdB, 1, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope340, DoAslW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope348, DoLslW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope350, DoRoxlW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope358, DoRolW, DW, Ss, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope360, DoAslW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope368, DoLslW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope370, DoRoxlW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope378, DoRolW, DW, SdW, 1, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope380, DoAslL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope388, DoLslL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope390, DoRoxlL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope398, DoRolL, DL, Ss, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope3a0, DoAslL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope3a8, DoLslL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope3b0, DoRoxlL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope3b8, DoRolL, DL, SdL, 1, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope400, DoAsrB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope408, DoLsrB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope410, DoRoxrB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope418, DoRorB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope420, DoAsrB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope428, DoLsrB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope430, DoRoxrB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope438, DoRorB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope440, DoAsrW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope448, DoLsrW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope450, DoRoxrW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope458, DoRorW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope460, DoAsrW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope468, DoLsrW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope470, DoRoxrW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope478, DoRorW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope480, DoAsrL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope488, DoLsrL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope490, DoRoxrL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope498, DoRorL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope4a0, DoAsrL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope4a8, DoLsrL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope4b0, DoRoxrL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope4b8, DoRorL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope500, DoAslB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope508, DoLslB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope510, DoRoxlB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope518, DoRolB, DB, Ss, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope520, DoAslB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope528, DoLslB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope530, DoRoxlB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope538, DoRolB, DB, SdB, 2, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope540, DoAslW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope548, DoLslW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope550, DoRoxlW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope558, DoRolW, DW, Ss, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope560, DoAslW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope568, DoLslW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope570, DoRoxlW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope578, DoRolW, DW, SdW, 2, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope580, DoAslL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope588, DoLslL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope590, DoRoxlL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope598, DoRolL, DL, Ss, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope5a0, DoAslL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope5a8, DoLslL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope5b0, DoRoxlL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope5b8, DoRolL, DL, SdL, 2, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope600, DoAsrB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope608, DoLsrB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope610, DoRoxrB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope618, DoRorB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope620, DoAsrB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope628, DoLsrB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope630, DoRoxrB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope638, DoRorB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope640, DoAsrW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope648, DoLsrW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope650, DoRoxrW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope658, DoRorW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope660, DoAsrW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope668, DoLsrW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope670, DoRoxrW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope678, DoRorW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope680, DoAsrL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope688, DoLsrL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope690, DoRoxrL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope698, DoRorL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope6a0, DoAsrL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope6a8, DoLsrL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope6b0, DoRoxrL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope6b8, DoRorL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope700, DoAslB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope708, DoLslB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope710, DoRoxlB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope718, DoRolB, DB, Ss, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope720, DoAslB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope728, DoLslB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope730, DoRoxlB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope738, DoRolB, DB, SdB, 3, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope740, DoAslW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope748, DoLslW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope750, DoRoxlW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope758, DoRolW, DW, Ss, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope760, DoAslW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope768, DoLslW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope770, DoRoxlW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope778, DoRolW, DW, SdW, 3, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope780, DoAslL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope788, DoLslL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope790, DoRoxlL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope798, DoRolL, DL, Ss, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope7a0, DoAslL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope7a8, DoLslL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope7b0, DoRoxlL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope7b8, DoRolL, DL, SdL, 3, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope800, DoAsrB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope808, DoLsrB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope810, DoRoxrB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope818, DoRorB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope820, DoAsrB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope828, DoLsrB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope830, DoRoxrB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope838, DoRorB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope840, DoAsrW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope848, DoLsrW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope850, DoRoxrW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope858, DoRorW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope860, DoAsrW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope868, DoLsrW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope870, DoRoxrW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope878, DoRorW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope880, DoAsrL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope888, DoLsrL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope890, DoRoxrL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope898, DoRorL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope8a0, DoAsrL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope8a8, DoLsrL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope8b0, DoRoxrL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope8b8, DoRorL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Ope900, DoAslB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope908, DoLslB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope910, DoRoxlB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope918, DoRolB, DB, Ss, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope920, DoAslB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope928, DoLslB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope930, DoRoxlB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope938, DoRolB, DB, SdB, 4, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Ope940, DoAslW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope948, DoLslW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope950, DoRoxlW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope958, DoRolW, DW, Ss, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope960, DoAslW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope968, DoLslW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope970, DoRoxlW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope978, DoRolW, DW, SdW, 4, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Ope980, DoAslL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope988, DoLslL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope990, DoRoxlL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope998, DoRolL, DL, Ss, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope9a0, DoAslL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope9a8, DoLslL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope9b0, DoRoxlL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Ope9b8, DoRolL, DL, SdL, 4, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Opea00, DoAsrB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea08, DoLsrB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea10, DoRoxrB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea18, DoRorB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea20, DoAsrB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea28, DoLsrB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea30, DoRoxrB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea38, DoRorB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opea40, DoAsrW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea48, DoLsrW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea50, DoRoxrW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea58, DoRorW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea60, DoAsrW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea68, DoLsrW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea70, DoRoxrW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea78, DoRorW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opea80, DoAsrL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opea88, DoLsrL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opea90, DoRoxrL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opea98, DoRorL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeaa0, DoAsrL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeaa8, DoLsrL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeab0, DoRoxrL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeab8, DoRorL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Opeb00, DoAslB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb08, DoLslB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb10, DoRoxlB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb18, DoRolB, DB, Ss, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb20, DoAslB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb28, DoLslB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb30, DoRoxlB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb38, DoRolB, DB, SdB, 5, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opeb40, DoAslW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb48, DoLslW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb50, DoRoxlW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb58, DoRolW, DW, Ss, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb60, DoAslW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb68, DoLslW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb70, DoRoxlW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb78, DoRolW, DW, SdW, 5, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opeb80, DoAslL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeb88, DoLslL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeb90, DoRoxlL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeb98, DoRolL, DL, Ss, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeba0, DoAslL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeba8, DoLslL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opebb0, DoRoxlL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opebb8, DoRolL, DL, SdL, 5, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Opec00, DoAsrB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec08, DoLsrB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec10, DoRoxrB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec18, DoRorB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec20, DoAsrB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec28, DoLsrB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec30, DoRoxrB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec38, DoRorB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opec40, DoAsrW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec48, DoLsrW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec50, DoRoxrW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec58, DoRorW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec60, DoAsrW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec68, DoLsrW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec70, DoRoxrW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec78, DoRorW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opec80, DoAsrL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opec88, DoLsrL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opec90, DoRoxrL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opec98, DoRorL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeca0, DoAsrL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeca8, DoLsrL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opecb0, DoRoxrL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opecb8, DoRorL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Oped00, DoAslB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped08, DoLslB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped10, DoRoxlB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped18, DoRolB, DB, Ss, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped20, DoAslB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped28, DoLslB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped30, DoRoxlB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped38, DoRolB, DB, SdB, 6, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Oped40, DoAslW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped48, DoLslW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped50, DoRoxlW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped58, DoRolW, DW, Ss, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped60, DoAslW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped68, DoLslW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped70, DoRoxlW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped78, DoRolW, DW, SdW, 6, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Oped80, DoAslL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Oped88, DoLslL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Oped90, DoRoxlL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Oped98, DoRolL, DL, Ss, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeda0, DoAslL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeda8, DoLslL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opedb0, DoRoxlL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opedb8, DoRolL, DL, SdL, 6, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Opee00, DoAsrB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee08, DoLsrB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee10, DoRoxrB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee18, DoRorB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee20, DoAsrB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee28, DoLsrB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee30, DoRoxrB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee38, DoRorB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opee40, DoAsrW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee48, DoLsrW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee50, DoRoxrW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee58, DoRorW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee60, DoAsrW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee68, DoLsrW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee70, DoRoxrW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee78, DoRorW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opee80, DoAsrL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opee88, DoLsrL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opee90, DoRoxrL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opee98, DoRorL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeea0, DoAsrL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeea8, DoLsrL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeeb0, DoRoxrL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opeeb8, DoRorL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)

OperS(Opef00, DoAslB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef08, DoLslB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef10, DoRoxlB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef18, DoRolB, DB, Ss, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef20, DoAslB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef28, DoLslB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef30, DoRoxlB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef38, DoRolB, DB, SdB, 7, DB, DR, Cd, ins7, GRB, SRB, 8)
OperS(Opef40, DoAslW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef48, DoLslW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef50, DoRoxlW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef58, DoRolW, DW, Ss, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef60, DoAslW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef68, DoLslW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef70, DoRoxlW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef78, DoRolW, DW, SdW, 7, DW, DR, Cd, ins7, GRW, SRW, 8)
OperS(Opef80, DoAslL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opef88, DoLslL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opef90, DoRoxlL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opef98, DoRolL, DL, Ss, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opefa0, DoAslL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opefa8, DoLslL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opefb0, DoRoxlL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)
OperS(Opefb8, DoRolL, DL, SdL, 7, DL, DR, Cd, ins7, GRL, SRL, 10)

Oper (Ope0d0, DoAsrW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope0d8, DoAsrW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope0e0, DoAsrW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope0e8, DoAsrW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope0f0, DoAsrW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope0f8, DoAsrW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope1d0, DoAslW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope1d8, DoAslW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope1e0, DoAslW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope1e8, DoAslW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope1f0, DoAslW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope1f8, DoAslW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope2d0, DoLsrW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope2d8, DoLsrW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope2e0, DoLsrW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope2e8, DoLsrW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope2f0, DoLsrW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope2f8, DoLsrW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope3d0, DoLslW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope3d8, DoLslW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope3e0, DoLslW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope3e8, DoLslW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope3f0, DoLslW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope3f8, DoLslW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope4d0, DoRoxrW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope4d8, DoRoxrW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope4e0, DoRoxrW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope4e8, DoRoxrW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope4f0, DoRoxrW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope4f8, DoRoxrW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope5d0, DoRoxlW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope5d8, DoRoxlW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope5e0, DoRoxlW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope5e8, DoRoxlW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope5f0, DoRoxlW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope5f8, DoRoxlW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope6d0, DoRorW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope6d8, DoRorW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope6e0, DoRorW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope6e8, DoRorW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope6f0, DoRorW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope6f8, DoRorW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

Oper (Ope7d0, DoRolW, DW, Ss, 1, DW, DA, Cain, ins7, GMW, SMW, 12)
Oper (Ope7d8, DoRolW, DW, Ss, 1, DW, DA, CaipW, ins7, GMW, SMW, 12)
Oper (Ope7e0, DoRolW, DW, Ss, 1, DW, DA, CmaiW, ins7, GMW, SMW, 16)
Oper (Ope7e8, DoRolW, DW, Ss, 1, DW, DA, Fdai, ins7, GMW, SMW, 16)
Oper (Ope7f0, DoRolW, DW, Ss, 1, DW, DA, Faix, ins7, GMW, SMW, 20)
Oper (Ope7f8, DoRolW, DW, Ss, 1, DW, DA, Feaw, ins7, GMW, SMW, 18)

