/*
 * proto.h - FAST 68000 emulator instruction prototypes
 * Version 1.0
 * Copyright (C) 1994, 1995 Joachim H�nig
 * (hoenig@informatik.uni-erlangen.de)
 *
 * This file is part of FAST, the Fine Atari ST Emulator.
 *
 * FAST is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * FAST is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FAST; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Prototypes of all 68000 opcodes defined in op68k*.c
 */

#undef Oper
#define Oper(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
    extern void a1();

#undef OperS
#define OperS(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
    extern void a1();

#define PROTOH

#include "obj/op68kadd_expanded.c"
#include "obj/op68karith_expanded.c"
#include "obj/op68ksub_expanded.c"
#include "obj/op68klogop_expanded.c"
#include "obj/op68kmisc_expanded.c"
#include "obj/op68kmove_expanded.c"
#include "obj/op68kshift_expanded.c"

extern void     Op4840();   /* SWAP */
extern void     Op4848();   /* BKPT */
extern void     Op4880();   /* EXT.W */
extern void     Op4890();   /* MOVEM.W to (ax) */
extern void     Op48a0();   /* MOVEM.W to -(ax) */
extern void     Op48a8();   /* MOVEM.W to d(ax) */
extern void     Op48b0();   /* MOVEM.W to d(ax,rx) */
extern void     Op48b8();   /* MOVEM.W to w l */
extern void     Op48c0();   /* EXT.L */
extern void     Op48d0();   /* MOVEM.L to (ax) */
extern void     Op48e0();   /* MOVEM.L to -(ax) */
extern void     Op48e8();   /* MOVEM.L to d(ax) */
extern void     Op48f0();   /* MOVEM.L to d(ax,rx) */
extern void     Op48f8();   /* MOVEM.L to w l */
extern void     Op4c90();   /* MOVEM.W from (ax) */
extern void     Op4c98();   /* MOVEM.W from (ax)+ */
extern void     Op4ca8();   /* MOVEM.W from d(ax) */
extern void     Op4cb0();   /* MOVEM.W from d(ax,rx) */
extern void     Op4cb8();   /* MOVEM.W from ea */
extern void     Op4cd0();   /* MOVEM.L from (ax) */
extern void     Op4cd8();   /* MOVEM.L from (ax)+ */
extern void     Op4ce8();   /* MOVEM.L from d(ax) */
extern void     Op4cf0();   /* MOVEM.L from d(ax,rx) */
extern void     Op4cf8();   /* MOVEM.L from ea */
extern void     Op4e40();   /* TRAP 0 */
extern void     Op4e41();   /* TRAP 1 */
extern void     Op4e42();   /* TRAP 2 */
extern void     Op4e43();   /* TRAP 3 */
extern void     Op4e44();   /* TRAP 4 */
extern void     Op4e45();   /* TRAP 5 */
extern void     Op4e46();   /* TRAP 6 */
extern void     Op4e47();   /* TRAP 7 */
extern void     Op4e48();   /* TRAP 8 */
extern void     Op4e49();   /* TRAP 9 */
extern void     Op4e4a();   /* TRAP 10 */
extern void     Op4e4b();   /* TRAP 11 */
extern void     Op4e4c();   /* TRAP 12 */
extern void     Op4e4d();   /* TRAP 13 */
extern void     Op4e4e();   /* TRAP 14 */
extern void     Op4e4f();   /* TRAP 15 */
extern void     Op4e50();   /* LINK */
extern void     Op4e58();   /* UNLK */
extern void     Op4e60();   /* MOVE ax,usp */
extern void     Op4e68();   /* MOVE usp,ax */
extern void     Op4e70();   /* RESET, NOP, STOP, RTE,
                                 * RTD, RTS, TRAPV, RTR */
extern void     Op4e78();   /* MOVEC */

extern void     Opc140();   /* EXG */
extern void     Opc148();   /* EXG */
extern void     Opc188();   /* EXG */
extern void     Opc340();   /* EXG */
extern void     Opc348();   /* EXG */
extern void     Opc388();   /* EXG */
extern void     Opc540();   /* EXG */
extern void     Opc548();   /* EXG */
extern void     Opc588();   /* EXG */
extern void     Opc740();   /* EXG */
extern void     Opc748();   /* EXG */
extern void     Opc788();   /* EXG */
extern void     Opc940();   /* EXG */
extern void     Opc948();   /* EXG */
extern void     Opc988();   /* EXG */
extern void     Opcb40();   /* EXG */
extern void     Opcb48();   /* EXG */
extern void     Opcb88();   /* EXG */
extern void     Opcd40();   /* EXG */
extern void     Opcd48();   /* EXG */
extern void     Opcd88();   /* EXG */
extern void     Opcf40();   /* EXG */
extern void     Opcf48();   /* EXG */
extern void     Opcf88();   /* EXG */
