# ------------------------------------
# Castaway/Atari
# ------------------------------------
APPLICATION = ../castaway.prg
CC = m68k-atari-mint-gcc
GEN = tools/jmptable

# ------------------------------------
# options
# ------------------------------------

ifeq ($(target),)
target=atari
endif

ifeq ($(target),atari)
TARGETDEFS = -DATARI
CPUFLAG=-m68020-60
CPULINK=-m68000

else ifeq ($(target),firebee)
TARGETDEFS = -DATARI -DFIREBEE -DCOLDFIRE
CPUFLAG=-mcpu=5475
CPULINK=-mcpu=5475

else ifeq ($(target),vampire)
TARGETDEFS = -DATARI -DVAMPIRE -DAC68080
CPUFLAG=-m68060
CPULINK=-m68060

else ifeq ($(target),raven)
TARGETDEFS = -DATARI -DRAVEN
CPUFLAG=-m68060
CPULINK=-m68000

else ifeq ($(target),fakefirebee)
TARGETDEFS = -DATARI -DCOLDFIRE
CPUFLAG=-m68020-60
CPULINK=-m68000

endif

ifeq ($(build),dist)
TARGETDEFS += -DDISTBUILD
else ifeq ($(build),debug)
TARGETDEFS += -DDEBUGBUILD
else
TARGETDEFS += -DRELEASEBUILD
endif

TARGETDEFS += $(defs)

CFLAGS = \
	$(TARGETDEFS) -DNO_ZLIB \
	-O3 \
	-std=c99 -fsigned-char -finline-functions -fomit-frame-pointer -fshort-enums -fno-common \
	-Wall -Wno-multichar -Wno-unused-variable -Wno-implicit-function-declaration -Wno-pointer-sign \
	-Wl,--traditional-format -Wl,-Map,mapfile \
	-ffixed-a6 \
	-ffixed-a5 \
	-ffixed-a4 \
	-ffixed-d7 \
	-I. -Iv4 -Icpu -Ist 

#usp			sp
#a6				membase
#a5		emu-pc	emu-pc
#d7		emu-ccr	emu-ccr
#d6				emu-inst
#d5				emu-cycles

#	-ffixed-a6
#	-ffixed-d6
#	-ffixed-d5

#LFLAGS = -lgcc -lgem $(CPUFLAG)
LFLAGS = -lgcc -lgem


# ------------------------------------
# source code
# ------------------------------------

OBJ_CPU_OPHANDLERS = op68kadd.o op68karith.o op68klogop.o op68kmisc.o op68kmove.o op68kshift.o op68ksub.o
OBJ_CPU = 68000.o op68000.o $(OBJ_CPU_OPHANDLERS)
OBJ_MCH = init.o st.o mem.o ikbd.o blitter.o fdc.o
OBJ_APP = emu.o host.o vampire.o nova.o start.o asm.o

OBJSNOPATH = $(OBJ_CPU) $(OBJ_MCH) $(OBJ_APP)
OBJS = $(OBJSNOPATH:%=obj/%)

LINKOBJS = \
		obj/op68000.o \
		obj/emu.o \
		obj/st.o \
		obj/op68kmove.o \
		obj/op68ksub.o \
		obj/op68kadd.o \
		obj/op68kmisc.o \
		obj/op68kshift.o \
		obj/op68klogop.o \
		obj/68000.o \
		obj/op68karith.o \
		obj/host.o \
		obj/start.o \
		obj/asm.o \
		obj/vampire.o \
		obj/nova.o \
		obj/fdc.o \
		obj/ikbd.o \
		obj/init.o \
		obj/blitter.o \
		obj/mem.o \


# ------------------------------------
# build targets
# ------------------------------------
.PHONY: linkonly

all: $(APPLICATION)

# ------------------------------------
# rules
# ------------------------------------
GLOBALDEPENDS=Makefile_atari.mak config.h cpu/op68k.h cpu/68000.h cpu/proto.h

$(APPLICATION): $(OBJS) $(GLOBALDEPENDS)
	$(CC) $(CPULINK) $(CFLAGS) -o $@ $(LINKOBJS) $(LFLAGS)
	tools/stripx -v -f -s $(APPLICATION)

obj/op68k%.o : cpu/op68k%.c $(GLOBALDEPENDS) tools
	$(GEN) $< $(@:%.o=%_expanded.c)
	$(CC) $(CPUFLAG) $(CFLAGS) -S $(@:%.o=%_expanded.c) -o $(@:%.o=%.0.s)
	$(GEN) $(@:%.o=%.0.s) $(@:%.o=%.1.s)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $(@:%.o=%.1.s) -o $@

obj/op68000.o : cpu/op68000.c $(OBJ_CPU_OPHANDLERS:%=obj/%) $(GLOBALDEPENDS) tools
	$(GEN) $< $(@:%.o=%_expanded.c)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $(@:%.o=%_expanded.c) -o $@

obj/68000.o : cpu/68000.c obj/op68000.o $(GLOBALDEPENDS)
	$(CC) $(CPUFLAG) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : st/%.c $(GLOBALDEPENDS)
	$(CC) $(CPUFLAG) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : %.c $(GLOBALDEPENDS)
	$(CC) $(CPUFLAG) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : %.S $(GLOBALDEPENDS)
	$(CC) $(CPUFLAG) $(CFLAGS) -c $< -o $@

linkonly:
	$(CC) $(CPULINK) $(CFLAGS) -o $(APPLICATION) $(LINKOBJS) $(LFLAGS)
	tools/stripx -v -f -s $(APPLICATION)
