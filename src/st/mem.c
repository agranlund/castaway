/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - memory read/write
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  30.05.2002  JH  Discontinued using mmap and mprotect, now using
*                  Martin's memory access jump table.
*  12.06.2002  JH  Correct bus error/address error exception stack frame
*  14.06.2002  JH  LowRamSetX() functions improved.
*  09.07.2002  JH  Now loads any 192k ROM file
*  10.07.2002  MAD Now loads any ROM file
*  16.09.2002  JH  Bugfix: Word access on unmapped I/O address stacked
*                  two bus error stack frames. Fault address corrected.
*  08.10.2002  JH  Fixed integer types.
*  27.10.2002  AG  Trashed everything for more speed! mwuhahaha!
*/

#include "config.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include "68000.h"
#include "st.h"
#include "mem.h"

extern short mach_raven;

//static const int samvol[16]={0,0,0,1,1,1,2,3,5,7,11,17,25,38,57,85};
#define CARTBASE    0xFA0000L
#define CARTSIZE    0x020000L
#define TOS1BASE    0xFC0000L
#define TOS1SIZE    0x030000L
#define TOS2BASE    0xE00000L
#define TOS2SIZE    0x040000L


#ifndef MEM_INLINE_ACCESS


char GetMemBpc(unsigned long address) {
    return ReadB(address);
}

short GetMemWpc(unsigned long address) {
    return ReadW(address);
}

long GetMemLpc(unsigned long address) {
    return ReadL(address);
}

char GetMemB(unsigned long address) {
	address &= MEMADDRMASK;
    if (address >= IOBASE)
		return DoIORB(address);
    return ReadB(address + membase);
}

short GetMemW(unsigned long address) {
    address &= MEMADDRMASK;
    CHECK_ADDR_ERR();
    if (address >= IOBASE)
        return DoIORW(address);
    return ReadW(address + membase);
}

long GetMemL(unsigned long address) {
    address &= MEMADDRMASK;
    CHECK_ADDR_ERR();
    if (address >= IOBASE)
		return DoIORL(address);
    return ReadL(address + membase);
}


short GetMemPW(unsigned long address) {
    register uint16 ret;
    address &= MEMADDRMASK;
    CHECK_READ_ADDR_ERR();
    if (address < IOBASE) {
        address += (uint32) membase;
        ret  = (uint8) ReadB(address + 0); ret <<= 8;
        ret |= (uint8) ReadB(address + 2);
    } else {
        ret  = (uint8) DoIORB(address + 0); ret <<= 8;
        ret |= (uint8) DoIORB(address + 2);
    }
    return ret;
}

long GetMemPL(unsigned long address) {
    register uint32 ret;
    address &= MEMADDRMASK;
    CHECK_READ_ADDR_ERR();
    if (address < IOBASE) {
        address += (uint32) membase;
        ret  = (uint8) ReadB(address + 0); ret <<= 8;
        ret |= (uint8) ReadB(address + 2); ret <<= 8;
        ret |= (uint8) ReadB(address + 4); ret <<= 8;
        ret |= (uint8) ReadB(address + 6);
    } else {
        ret  = (uint8) DoIORB(address + 0); ret <<= 8;
        ret |= (uint8) DoIORB(address + 2); ret <<= 8;
        ret |= (uint8) DoIORB(address + 4); ret <<= 8;
        ret |= (uint8) DoIORB(address + 6);
    }
    return ret;
}

void SetMemB (unsigned long address, unsigned char value) {
    address &= MEMADDRMASK;
	ON_WRITE(address, value);
	if (address<MEMSIZE) {
        CHECK_SUPERV_WRITE();
        CHECK_ZERO_WRITE();
		WriteB(address + membase, value);
		return;
	}
	if (address>=IOBASE) {
		DoIOWB(address, value);
		return;
	}
	ON_UNMAPPED(address, value);
}

void SetMemW(unsigned long address, unsigned short value)
{	
    address &= MEMADDRMASK;
    CHECK_WRITE_ADDR_ERR();
	ON_WRITE(address, value);
	if (address<MEMSIZE) {
        CHECK_SUPERV_WRITE();
        CHECK_ZERO_WRITE();
		WriteW(address + membase, value);
		return;
	}
	if (address>=IOBASE) {
		DoIOWW(address, value);
		return;
	}
	ON_UNMAPPED(address, value);
}

void SetMemL(unsigned long address, unsigned long value)
{
    //DBG("SetMemL %08x %08x", address, value);
    address &= MEMADDRMASK;
    CHECK_WRITE_ADDR_ERR();
	ON_WRITE(address, value);
	if (address<MEMSIZE) {
        CHECK_SUPERV_WRITE();
        CHECK_ZERO_WRITE();
		WriteL(address + membase, value);
		return;
	}
	if (address>=IOBASE) {
		DoIOWL(address, value);
		return;
	}
	ON_UNMAPPED(address, value);
}


void SetMemPW(unsigned long address, unsigned long value)
{
    address &= MEMADDRMASK;
    if (address>=IOBASE && address<IOBASE+IOSIZE) {
        DoIOWB(address, (int8)(value>>8));
        DoIOWB(address+2, (int8)value);
        return;
    }
    if (address<MEMSIZE) {
        if (address<SVADDR && !GetFC2()){
            ExceptionGroup0(BUSERR, address, 0);
            return;
        }
        address+=(uint32)membase;
        WriteB(address, (int8)(value>>8));
        WriteB(address+2, (int8)value);
        return;
    }
    ON_UNMAPPED(address, value);
}

void SetMemPL(unsigned long address, unsigned long value)
{
    address &= MEMADDRMASK;
    if (address>=IOBASE && address<IOBASE+IOSIZE) {
        //IO
        DoIOWB(address, (int8)(value>>24));
        DoIOWB(address+2, (int8)(value>>16));
        DoIOWB(address+4, (int8)(value>>8));
        DoIOWB(address+6, (int8)value);
        return;
    }
    if (address<MEMSIZE) {
        //RAM
        if (address<SVADDR && !GetFC2()){
            ExceptionGroup0(BUSERR, address, 0);
            return;
        }
        address+=(uint32)membase;
        WriteB(address, (int8)(value>>24));
        WriteB(address+2, (int8)(value>>16));
        WriteB(address+4, (int8)(value>>8));
        WriteB(address+6, (int8)value);
        return;
    }
    ON_UNMAPPED(address, value);
}

#endif


int MemInit(void)
{

	if (mach_raven) {
		// use 24bit emulator space
    	cpup->membase = membase = (int8*) 0x04000000;
	} else {
		// allocate 16mb (aligned at 16mb) for ST address map
		cpup->membase = membase = (int8*) AllocateMem(0x01000000, 0x01000000, MEM_FAST);
	}

    if (!membase) {

        DBG("Failed allocating ST mem");
        return 0;
    }
    DBG("membase = %08x", (uint32)membase);
    memset (membase, 0, 0x01000000);
    FILE *romf = fopen(ROM, "rb");
	if (romf == NULL) {
        DBG("Failed to open rom");
        return 0;
	}

    uint8* rombase = membase + TOS2BASE;
    uint32 romsize = fread(rombase, 1, TOS2SIZE, romf);
    fclose(romf);
    if (romsize == TOS1SIZE) {
        rombase = membase + TOS1BASE;
        memcpy(rombase, membase + TOS2BASE, TOS1SIZE);
    }

	memcpy (membase, rombase, 8);
    memset (membase + CARTBASE, 0xff, CARTSIZE);
    return 1;
}
