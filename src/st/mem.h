#ifndef _mem_h_
#define _mem_h_
#include "config.h"
#include "68000.h"

#define MEM_INLINE_ACCESS

//register unsigned long MEMADDRMASK __asm__("d4");
#define MEMADDRMASK 0x00ffffffl


#ifdef LITTLE_ENDIAN
#define ReadB(addr)  *(uint8*)((uint32)(addr)^1)
#define ReadW(addr)  *(uint16*)(addr) 
#define ReadL(addr)  ((*(uint16*)(addr))<<16)|(*(uint16*)(addr+2)) 
#define ReadSL(addr) (*(uint16*)(addr))|((*(uint16*)(addr+2))<<16)
#define WriteB(addr,value) *(uint8*)((uint32)(addr)^1)=value 
#define WriteW(addr,value) *(uint16*)(addr)=value 
#define WriteL(addr,value) WriteW(addr + 2, value); WriteW(addr, value>> 16)
#else
#define ReadB(addr)  (uint8)((*((uint8*)(addr))))
#define ReadW(addr)  (uint16)((*((uint16*)(addr))))
#define ReadL(addr)  (uint32)((*((uint32*)(addr))))
#define ReadSL(addr) ((*((uint16*)(addr)))|((*(uint16*)((addr)+2))<<16))
#define WriteB(addr,value) *((uint8*)(addr))=value 
#define WriteW(addr,value) *((uint16*)(addr))=value 
#define WriteL(addr,value) *((uint32*)(addr))=value
#endif


#ifdef CHKADDRESSERR
#define CHECK_READ_ADDR_ERR()   if (address & 0x1){ ExceptionGroup0(ADDRESSERR, address, 1); return 0; }
#define CHECK_WRITE_ADDR_ERR()  if (address & 0x1){ /*ExceptionGroup0(ADDRESSERR, address, 1);*/ return; }
#define CHECK_SUPERV_WRITE()    if (address<SVADDR && !GetFC2()) { ExceptionGroup0(BUSERR, address, 0); return; }
#define CHECK_ZERO_WRITE()      if (address<8) { ExceptionGroup0(BUSERR, address, 0); return; }
#else
#define CHECK_READ_ADDR_ERR()
#define CHECK_WRITE_ADDR_ERR()
#define CHECK_SUPERV_WRITE()
#define CHECK_ZERO_WRITE()
#endif



#ifdef MEM_INLINE_ACCESS

extern uint8 DoIORB(uint32 address);
extern uint16 DoIORW(uint32 address);
extern uint32 DoIORL(uint32 address);
extern void DoIOWB(uint32 address, uint8 value);
extern void DoIOWW(uint32 address, uint16 value);
extern void DoIOWL(uint32 address, uint32 value);

// read mem pc relative (address in host space)
static inline char GetMemBpc(unsigned long address) {
    return ReadB(address);
}

static inline short GetMemWpc(unsigned long address) {
    return ReadW(address);
}

static inline long GetMemLpc(unsigned long address) {
    return ReadL(address);
}

// read mem (address in guest space)
static inline char GetMemB(unsigned long address) {
	address &= MEMADDRMASK;
    if (address >= IOBASE)
		return DoIORB(address);
    return ReadB(address + membase);
}

static inline short GetMemW(unsigned long address) {
    address &= MEMADDRMASK;
    CHECK_READ_ADDR_ERR();
    if (address >= IOBASE)
        return DoIORW(address);
    return ReadW(address + membase);
}

static inline long GetMemL(unsigned long address) {
    address &= MEMADDRMASK;
    CHECK_READ_ADDR_ERR();
    if (address >= IOBASE)
		return DoIORL(address);
    return ReadL(address + membase);
}

static inline short GetMemPW(unsigned long address) {
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

static inline long GetMemPL(unsigned long address) {
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

static inline void SetMemB (unsigned long address, unsigned char value) {
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

static inline void SetMemW(unsigned long address, unsigned short value)
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

static inline void SetMemL(unsigned long address, unsigned long value)
{
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

static inline void SetMemPW(unsigned long address, unsigned long value)
{
    address &= MEMADDRMASK;
    CHECK_WRITE_ADDR_ERR();
	ON_WRITE(address, value);
	if (address<MEMSIZE) {
        CHECK_SUPERV_WRITE();
        CHECK_ZERO_WRITE();
        address+=(uint32)membase;
        WriteB(address, (int8)(value>>8));
        WriteB(address+2, (int8)value);
        return;
    }
    if (address >= IOBASE) {
        DoIOWB(address, (int8)(value>>8));
        DoIOWB(address+2, (int8)value);
        return;
    }
    ON_UNMAPPED(address, value);
}

static inline void SetMemPL(unsigned long address, unsigned long value)
{
    address &= MEMADDRMASK;
    CHECK_WRITE_ADDR_ERR();
	if (address<MEMSIZE) {
        CHECK_SUPERV_WRITE();
        CHECK_ZERO_WRITE();
        address+=(uint32)membase;
        WriteB(address+0, (int8)(value>>24));
        WriteB(address+2, (int8)(value>>16));
        WriteB(address+4, (int8)(value>>8));
        WriteB(address+6, (int8)value);
        return;
    }
    if (address >= IOBASE) {
        DoIOWB(address+0, (int8)(value>>24));
        DoIOWB(address+2, (int8)(value>>16));
        DoIOWB(address+4, (int8)(value>>8));
        DoIOWB(address+6, (int8)value);
        return;
    }
    ON_UNMAPPED(address, value);
}

#else

extern char GetMemBpc(unsigned long address);
extern short GetMemWpc(unsigned long address);
extern long GetMemLpc(unsigned long address);

extern char GetMemB(unsigned long address);
extern short GetMemW(unsigned long address);
extern long GetMemL(unsigned long address);

extern void SetMemB(unsigned long address, unsigned char value);
extern void SetMemW(unsigned long address, unsigned short value);
extern void SetMemL(unsigned long address, unsigned long value);

extern void SetMemB(unsigned long address, unsigned char value);
extern void SetMemW(unsigned long address, unsigned short value);
extern void SetMemL(unsigned long address, unsigned long value);

#endif

#endif //_mem_h_
