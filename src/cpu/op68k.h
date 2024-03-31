/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */
#ifndef OP68KH
#define OP68KH

#include "config.h"
#include "68000.h"
#include "mem.h"


register unsigned short inst __asm__("d6");
register short cyc __asm__("d5");


#define ReadB(addr)  (uint8)((*((uint8*)(addr))))
#define ReadW(addr)  (uint16)((*((uint16*)(addr))))
#define ReadL(addr)  (uint32)((*((uint32*)(addr))))
#define ReadSL(addr) ((*((uint16*)(addr)))|((*(uint16*)((addr)+2))<<16))
#define WriteB(addr,value) *((uint8*)(addr))=value 
#define WriteW(addr,value) *((uint16*)(addr))=value 
#define WriteL(addr,value) *((uint32*)(addr))=value

/*
 * Status Register Access
 */

#define Cflag   0x1U
#define Vflag   0x2U
#define Zflag   0x4U
#define Nflag   0x8U
#define Xflag   0x10U
#define Sflag   0x2000U
#define Tflag   0x8000U

#define SetC(flag) { if (flag) { ccr |= 0x1U ; } else { ccr &= ~0x1U; } }
#define SetV(flag) { if (flag) { ccr |= 0x2U ; } else { ccr &= ~0x2U; } }
#define SetZ(flag) { if (flag) { ccr |= 0x4U ; } else { ccr &= ~0x4U; } }
#define SetZI(flag) { if (flag) { ccr &= ~0x4U ; } else { ccr |= 0x4U; } }
#define SetN(flag) { if (flag) { ccr |= 0x8U ; } else { ccr &= ~0x8U; } }
#define SetX(flag) { if (flag) { ccr |= 0x10U; } else { ccr &= ~0x10U; } }
#define SetXC(flag) { if (flag) { ccr |= 0x11U; } else { ccr &= ~0x11U; } }

#define ForceC SetC
#define ForceV SetV
#define ForceZ SetZ
#define ForceN SetN
#define ForceX SetX
#define ForceCVNZ(operand) { ccr &= ~0xfU; ccr |= (operand & 0xfU); }

#define SetNZ(operand) { \
    ccr &= ~0xcU; \
    if (operand == 0) { ccr |= Zflag; } \
    else if (operand < 0) { ccr |= Nflag; } \
}

#define ClrCV() { ccr &= ~(Cflag|Vflag); }

#define ZeroZ(flag) { if (flag) ccr &= ~Zflag; }

#define ClrCVSetNZ(operand) { \
    ccr &= ~0xfU; \
    if (operand == 0) ccr |= Zflag; \
    else if (operand < 0) ccr |= Nflag; \
}

#define SetS(flag) { \
    if (cpup->status & Sflag) { cpup->ssp = cpup->reg[15]; } else { cpup->usp = cpup->reg[15]; } \
    if (flag) { cpup->status |= Sflag; cpup->reg[15] = cpup->ssp; } else { cpup->status &= ~Sflag; cpup->reg[15] = cpup->usp; } \
}

#define SetT(flag) { if (flag) cpup->status |= Tflag; else cpup->status &= ~Tflag; }

#define SetI(imask) { cpup->status = ((cpup->status & 0xf8ffU) | (((imask)<<8) & 0x0700U)); cpup->recalc_int = 1; }

#define GetC() (!!(ccr&0x1))
#define GetV() (!!(ccr&0x2))
#define GetZ() (!!(ccr&0x4))
#define GetN() (!!(ccr&0x8))
#define GetX() (!!(ccr&0x10))

#define GetS() (!!(cpup->status&0x2000))
#define GetT() (!!(cpup->status&0x8000))
#define GetI() ((cpup->status&0x0700)>>8)

/*
#define GetS() (!!(cpup->status&0x2000))
#define GetT() (!!(cpup->status&0x8000))
#define GetI() ((cpup->status&0x0700)>>8)
*/

static inline void SetSRB(unsigned short sr) {
    ccr = sr;
}

static inline void SetSRW(unsigned short sr) {
    cpup->status = ccr = sr;
    cpup->ssp = cpup->reg[15];
    if ((cpup->status & Sflag) == 0)
        cpup->reg[15] = cpup->usp;
    cpup->recalc_int = 1;
}

static inline unsigned short GetSRB() {
    return ccr;
}

static inline unsigned short GetSRW() {
    unsigned short ret = cpup->status;
    __asm__ volatile ( "move.b  d7,%0\n\r" : "=d"(ret) : "0"(ret) : "cc" );
    return ret;
}

#define INSTR_BEGIN \
    "move  d7,ccr\n\t"

#define INSTR_END \
    "move  ccr,d7\n\t" \

#define INSTR_BEGIN_NCC

#define INSTR_END_NCC

#define ADD_CYCLES(val) {\
    __asm__ volatile ("sub.w %0,d5\n\t" : : "g"(val) : "cc" );\
}
#define ADDI_CYCLES(val) {\
    if (val <= 8) { __asm__ volatile ("subq.w %0,d5\n\t" : : "i"(val) : "cc" ); }\
    else { __asm__ volatile ("sub.w %0,d5\n\t" : : "i"(val) : "cc" ); }\
}


/*
 * Other Register Access
 */

#if 1
#define GetPC() (pc & MEMADDRMASK)
#define SetPC(addr) pc = (uint32)(((addr) & MEMADDRMASK) + membase)
#else
#define GetPC() pc
#define SetPC(addr) pc = addr
#endif
#define FastMPCW() ((int32)GetMPCW())


#define GetMPCB() GetMemBpc (pc + 1)
#define GetMPCW() GetMemWpc (pc)
#define GetMPCL() GetMemLpc (pc)

#define GetRegB(regno) (int8) *(((int8 *) &cpup->reg[regno]) + 3)
#define GetRegW(regno) (int16) *(((int16 *) &cpup->reg[regno]) + 1)
#define GetRegL(regno) (int32) cpup->reg[regno]

#define GetARegW(regno) (int16) *(((int16 *) &cpup->reg[regno+8]) + 1)
#define GetARegL(regno) (int32) cpup->reg[regno+8]

#define SetRegB(regno, value) *(((int8 *) &cpup->reg[regno]) + 3) = value
#define SetRegW(regno, value) *(((int16 *) &cpup->reg[regno]) + 1) = value
#define SetARegW(regno, value) *(((int16 *) &cpup->reg[regno+8]) + 1) = value

#define SetRegL(regno, value) cpup->reg[regno] = value
#define SetARegL(regno, value) cpup->reg[regno+8] = value



/*
 * Addressing modes (target)
 */
/* d, a, ain, aip, mai, dai, aix, imm, (eaw,ear,eac) */
#define CN(address,spec)
#define Cs(address,spec) address=spec;
#define Cd(address,spec) address=spec;
#define Ca(address,spec) address=8+spec;
#define Cain(address,spec) address=cpup->reg[spec+8];
#define CaipW(address,spec) address=cpup->reg[spec+8]; cpup->reg[spec+8]=address+2;
#define CaipL(address,spec) address=cpup->reg[spec+8]; cpup->reg[spec+8]=address+4;
#define CmaiW(address,spec) address=cpup->reg[spec+8]; address-=2; cpup->reg[spec+8]=address;
#define CmaiL(address,spec) address=cpup->reg[spec+8]; address-=4; cpup->reg[spec+8]=address;
#define Cdai(address,spec) address = cpup->reg[spec+8] + GetMPCW (); pc += 2;
#define Caix(address,spec) { register uint16 mod = GetMPCW(); pc += 2; address = cpup->reg[spec+8] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)); }
#define CaipB(address,spec) address=cpup->reg[spec+8]; if (spec==7) cpup->reg[spec+8]=address+2; else cpup->reg[spec+8]=address+1;
#define CaipB15(address,spec) address=cpup->reg[spec+8]; cpup->reg[spec+8]=address+2;
#define CmaiB(address,spec) address=cpup->reg[spec+8]; if (spec==7) address-=2; else address--; cpup->reg[spec+8]=address;
#define CmaiB15(address,spec) address=cpup->reg[spec+8]; address-=2; cpup->reg[spec+8]=address;

#define Ceaw(address, spec) \
switch (spec) {\
case 0x0:\
    address = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    address = GetMPCL ();\
    pc += 4;\
    ADDI_CYCLES(4);\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    address = 0;\
    break;\
}
#define Cear(address,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    address = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    address = GetMPCL ();\
    pc += 4;\
	ADDI_CYCLES(4);\
    break;\
case 0x2:\
    address = GetPC() + GetMPCW ();\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    address = GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12));\
    pc += 2;\
	ADDI_CYCLES(2);\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    address = 0;\
    break;\
}}

#define Fdai Cdai
#define Faix Caix
#define Feaw Ceaw
#define Fear Cear

/*
 * Operand types: B byte, W word, L long, R regno, A address
 */
#define DN(name)
#define DB(name) register int8 name;
#define DW(name) register int16 name;
#define DL(name) register int32 name;
#define DR(name) register uint16 name;
#define DA(name) register int32 name;


/*
 * target macros
 */
#define GRB(regno, name) name = GetRegB (regno);
#define GRW(regno, name) name = GetRegW (regno);
#define GRL(regno, name) name = GetRegL (regno);
#define SRB(regno, name) SetRegB (regno, name);
#define SRW(regno, name) SetRegW (regno, name);
#define SRL(regno, name) SetRegL (regno, name);
#define GMB(address, name) name = GetMemB (address);
#define GMW(address, name) name = GetMemW (address);
#define GML(address, name) name = GetMemL (address);
#define GN(address, name)
#define SMB(address, name) SetMemB (address, name);
#define SMW(address, name) SetMemW (address, name);
#define SML(address, name) SetMemL (address, name);
#define SN(address, name)
#define GCB(dummy, name) name = GetSRB ();
#define GCW(dummy, name) name = GetSRW ();
#define SCB(dummy, name) SetSRB (name);
#define SCW(dummy, name) SetSRW (name);
#define GPW(address, name) name = GetMemPW(address);
#define GPL(address, name) name = GetMemPL(address);
#define SPW(address, name) SetMemPW(address, name);
#define SPL(address, name) SetMemPL(address, name);


/*
 * source macros
 */
#define SdaiPW(var,spec) {uint32 a; Cdai(a,spec); GPW(a,var);}
#define SdaiPL(var,spec) {uint32 a; Cdai(a,spec); GPL(a,var);}
#define SNN(var,spec)
#define SNQ(var, dummy) var = (int8) inst;
#define ScB(var,spec) var = GetSRB();
#define ScW(var,spec) var = GetSRW();
#define Ss(var,spec) var = spec;
#define SimmB(var,spec) var = GetMPCB (); pc += 2;
#define SimmW(var,spec) var = GetMPCW (); pc += 2;
#define SimmL(var,spec) var = GetMPCL (); pc += 4;
#define SdB(var,spec) var = GetRegB (spec);
#define SdW(var,spec) var = GetRegW (spec);
#define SdL(var,spec) var = GetRegL (spec);
#define SaW(var,spec) var = GetARegW (spec);
#define SaL(var,spec) var = GetARegL (spec);
#define SainB(var,spec) var = GetMemB (cpup->reg[spec+8]);
#define SainW(var,spec) var = GetMemW (cpup->reg[spec+8]);
#define SainL(var,spec) var = GetMemL (cpup->reg[spec+8]);
#define SainA(var,spec) var = (cpup->reg[spec+8]);
#define SaipW(var,spec) {register uint32 myadr=cpup->reg[spec+8]; cpup->reg[spec+8]=myadr+2; var=GetMemW(myadr);}
#define SaipL(var,spec) {register uint32 myadr=cpup->reg[spec+8]; cpup->reg[spec+8]=myadr+4; var=GetMemL(myadr);}
#define SmaiW(var,spec) {register uint32 myadr=cpup->reg[spec+8]; myadr-=2; cpup->reg[spec+8]=myadr; var=GetMemW(myadr);}
#define SmaiL(var,spec) {register uint32 myadr=cpup->reg[spec+8]; myadr-=4; cpup->reg[spec+8]=myadr; var=GetMemL(myadr);}
#define SdaiB(var,spec) var = GetMemB (cpup->reg[spec+8] + GetMPCW ()); pc += 2;
#define SdaiW(var,spec) var = GetMemW (cpup->reg[spec+8] + GetMPCW ()); pc += 2;
#define SdaiL(var,spec) var = GetMemL (cpup->reg[spec+8] + GetMPCW ()); pc += 2;
#define SdaiA(var,spec) var = (cpup->reg[spec+8] + GetMPCW ()); pc += 2;
#define SaixB(var,spec) {register uint16 mod=GetMPCW(); pc+=2; var = GetMemB(cpup->reg[spec+8] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixW(var,spec) {register uint16 mod=GetMPCW(); pc+=2; var = GetMemW(cpup->reg[spec+8] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixL(var,spec) {register uint16 mod=GetMPCW(); pc+=2; var = GetMemL(cpup->reg[spec+8] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixA(var,spec) {register uint16 mod=GetMPCW(); pc+=2; var = (cpup->reg[spec+8] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
// these seem different from orgiginal core
#define SaipB(var,spec) {register uint32 myadr=cpup->reg[spec+8]; if (spec==7) cpup->reg[spec+8]=myadr+2; else cpup->reg[spec+8]=myadr+1; var=GetMemB(myadr);}
#define SmaiB(var,spec) {register uint32 myadr=cpup->reg[spec+8]; if (spec==7) myadr-=2; else myadr--; cpup->reg[spec+8]=myadr; var=GetMemB(myadr);}

#define SearB(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemB (GetMPCW ());\
    pc += 2;\
	ADDI_CYCLES(4);\
    break;\
case 0x1:\
    var = GetMemB (GetMPCL ());\
    pc += 4;\
	ADDI_CYCLES(8);\
    break;\
case 0x2:\
    var = GetMemBpc (pc + GetMPCW ()); \
    pc += 2;\
	ADDI_CYCLES(4);\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemBpc (pc + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
	ADDI_CYCLES(6);\
    break;\
case 0x4:\
    var = GetMPCB ();\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearW(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemW (GetMPCW ());\
    pc += 2;\
	ADDI_CYCLES(4);\
    break;\
case 0x1:\
    var = GetMemW (GetMPCL ());\
    pc += 4;\
	ADDI_CYCLES(8);\
    break;\
case 0x2:\
    var = GetMemWpc (pc + GetMPCW ());\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemWpc (pc + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
	ADDI_CYCLES(6);\
    break;\
case 0x4:\
    var = GetMPCW ();\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearL(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemL (GetMPCW ());\
    pc += 2;\
	ADDI_CYCLES(4);\
    break;\
case 0x1:\
    var = GetMemL (GetMPCL ());\
    pc += 4;\
	ADDI_CYCLES(8);\
    break;\
case 0x2:\
    var = GetMemLpc (pc + GetMPCW ());\
    pc += 2;\
	ADDI_CYCLES(4);\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemLpc (pc + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
	ADDI_CYCLES(6);\
    break;\
case 0x4:\
    var = GetMPCL ();\
    pc += 4;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearA(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    var = GetMPCL ();\
    pc += 4;\
	ADDI_CYCLES(4);\
    break;\
case 0x2:\
    var = GetPC() + GetMPCW ();\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12));\
    pc += 2;\
	ADDI_CYCLES(2);\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}

/*
 * Condition-Code queries
 */
#define CCt  0xff
#define CCf  0x00
#define CChi (!GetC() && !GetZ())
#define CCls (GetC() || GetZ())
#define CCcc !GetC()
#define CCcs GetC()
#define CCne !GetZ()
#define CCeq GetZ()
#define CCvc !GetV()
#define CCvs GetV()
#define CCpl !GetN()
#define CCmi GetN()
#define CCge !(GetN() ^ GetV())
#define CClt (GetN() ^ GetV())
#define CCgt (!(GetN() ^ GetV()) && !GetZ())
#define CCle ((GetN() ^ GetV()) || GetZ())

/*
 * Oper arguments:
 * Code                     Opcode
 * Op(target,source)        Macro used to execute operation
 * DeclS(name)              Macro to declare source (such as DB, DW or DL, or
                            DN=no source, DA=address, DR=register number)
 * GetS(var,spec)           Macro to fetch source, with addressing mode evaluation.
                            SNN = no fetch
                            SNQ = used in MOVEQ
                            ScB = condition code register
                            ScW = status register word (s-flag not checked)
                            Ss = source equals source specifier (spec1)
                            SaW, SaL = get #<spec1> data register value
                            SdB, SdW, SdL = get data register value
                            SainB, SainW, SainL = (A<spec>)
                            SainA = (LEA only)
                            SaixB, SaixW, SaixL = (d8,A<spec>,X<>)
                            SaixA = (LEA only)
                            SdaiB, SdaiW, SdaiL = d16(A<spec>)
                            SdaiA = (LEA only)
                            SaipB, SaipW, SaipL = (A<spec>)+
                            SmaiB, SmaiW, SmaiL = -(A<spec>)
                            SimmB, SimmW, SimmL = immediate
                            SdaiPW, SdaiPL = used in MOVEP
                            SearB, SearW, SearL = other addressing modes
                            SearA = (LEA only)
 * spec1                    source specifier, usually the data or address register number.
 * DeclT(name)              Macro to declare target (such as DB, DW or DL, or
                            DN=no target, DA=address, DR=register number)
 * DeclEA2(name)            Macro to declare target address (typically DA, DR, or DN)
 * CalcEA2(address,spec)    Macro to calculate address, with addressing mode evaluation.
                            CN = no address
                            Cs = equals target specifier (spec2)
                            Cd = equals target specifier (spec2) (data register number)
                            Ca = equals target specifier (spec2) + 8 (address register number)
                            Cain = (A<spec>)
                            Caix = (d8,A<spec>,X<>)
                            Cdai = d16(A<spec>)
                            CaipB, CaipW, CaipL = (A<spec>)+,
                            CaipB15 = stack pointer only
                            CmaiB, CmaiW, CmaiL = -(A<spec>)
                            CmaiB15 = stack pointer only
                            Ceaw = other addressing modes (if target is modified)
                            Cear = other addressing modes (if target is not modified)
 * spec2                    target specifier, usually the data or address register number.
 * GetEA2(address,name)     Macro to get target value
                            GCB = get condition code
                            GCW = get status (s-flag not checked)
                            GN = no read
                            GMB, GMW, GML = read from memory
                            GPW, GPL = (MOVEP)
                            GRB, GRW, GRL = read from register
 * SetEA2(address,name)     Macro to set target value
                            SCB = set condition code
                            SCW = set status (s-flag not checked)
                            SN = no write
                            SMB, SMW, SML = write to memory
                            SPW, SPL = (MOVEP)
                            SRB, SRW, SRL = write to register
 */


//
// This is all kinds of sketchy, but speedy.
// We're jumping into the opcode functions *after* the GCC-generated prologue,
// and exit them *before* the epilogue. Thus ignoring register save/restore.
// 
// EmulateCPU() is resposible for setting things up, and to adjust GCC's
// now broken view of which registers are clobbered.
// The USP register is (ab)used as a stack corrector on exit
//

#define STR(x) #x
#define XSTR(s) STR(s)

#define OperBegin(Code) \
    void C##Code() {\
    __asm__ volatile (\
    ".globl _"XSTR(Code)"\n\t"\
    "_"XSTR(Code)":\n\t" );

#if CHAINED_OPS
    extern void (*const jmp_table[8192*8])();
    #define OperBeginS(...) OperBegin(__VA_ARGS__)
    #define OperExitS(...)  OperExit(__VA_ARGS__)
    #ifndef COLDFIRE
        // m68k
        #define OperExit(retval) \
            ADDI_CYCLES(retval);\
            __asm__ volatile (\
            "ble.b   _OperChainDone%=\n\t"\
            "move.w  (a5)+,d6\n\t"\
            "jmp     ([_jmp_table+(4096*8*4),d6.w*4])\n\t"
            "_OperChainDone%=:\n\t"\
            "move.l  usp,sp\n\t"\
            "rts\n\t" : : : "cc" ); }
    #else
        // coldfire
        extern uint32 OperStackRestore;
        #define OperExit(retval) \
            ADDI_CYCLES(retval);\
            __asm__ volatile (\
            "ble.b   _OperChainDone%=\n\t"\
            "move.w  (a5)+,d6\n\t"\
            "move.l  #_jmp_table,a0\n\t"\
            "move.l  (a0,d6.l*4),a0\n\t"\
            "jmp     (a0)\n\t"
            "_OperChainDone%=:\n\t"\
            "move.l  _OperStackRestore,sp\n\t"\
            "rts\n\t" : : : "cc" ); }
    #endif
#else
    #define OperBeginS(Code) void Code() {
    #define OperExitS(retval) ADDI_CYCLES(retval); }
    #ifndef COLDFIRE
        // m68k
        #define OperExit(retval) \
            ADDI_CYCLES(retval);\
            __asm__ volatile (\
            "move.l usp,sp\n\t"\
            "rts\n\t" : : : "cc" ); }
    #else
        // coldfire
        extern uint32 OperStackRestore;
        #define OperExit(retval) \
            ADDI_CYCLES(retval);\
            __asm__ volatile (\
            "move.l _OperStackRestore,sp\n\t"\
            "rts\n\t" : : : "cc" ); }
    #endif
#endif


#define Oper(Code, Op, DeclS, GetS, spec1, DeclT, DeclEA2, CalcEA2, spec2, GetEA2, SetEA2, rval)\
OperBegin(Code)\
    DeclS (source)\
    DeclT (target)\
    DeclEA2 (address2)\
    GetS (source, spec1)\
    CalcEA2 (address2, spec2)\
    GetEA2 (address2, target)\
    Op (target, source)\
    SetEA2 (address2, target)\
    OperExit(rval)

#define OperS(Code, Op, DeclS, GetS, spec1, DeclT, DeclEA2, CalcEA2, spec2, GetEA2, SetEA2, rval)\
OperBeginS(Code)\
    DeclS (source)\
    DeclT (target)\
    DeclEA2 (address2)\
    GetS (source, spec1)\
    CalcEA2 (address2, spec2)\
    GetEA2 (address2, target)\
    Op (target, source)\
    SetEA2 (address2, target)\
    OperExitS(rval)

#endif //#define OP68KH
