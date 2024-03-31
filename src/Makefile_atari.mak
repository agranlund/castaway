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

else ifeq ($(target),firebee)
TARGETDEFS = -DATARI -DFIREBEE -DCOLDFIRE
CPUFLAG=-mcpu=5475

else ifeq ($(target),vampire)
TARGETDEFS = -DATARI -DVAMPIRE -DAC68080
CPUFLAG=-m68060

else ifeq ($(target),fakefirebee)
TARGETDEFS = -DATARI -DCOLDFIRE
CPUFLAG=-m68020-60

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
	-O3 $(CPUFLAG) \
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

LFLAGS = -lgcc -lgem $(CPUFLAG)


# ------------------------------------
# source code
# ------------------------------------

#OBJS =  cpu/debug.o cpu/op68kadd.o cpu/op68karith.o cpu/op68ksub.o cpu/op68klogop.o \
#        cpu/op68kmisc.o cpu/op68kmove.o cpu/op68kshift.o \
#		st/init.o st/st.o st/mem.o st/ikbd.o st/blitter.o st/fdc.o \
#		cpu/68000.o main.o atari.o amiga.o \

#OBJS =  atari.o amiga.o \
#        cpu/op68kmove.o cpu/op68kshift.o cpu/op68kmisc.o \
#		st/init.o st/st.o st/mem.o st/ikbd.o st/blitter.o st/fdc.o main.o \
#		cpu/debug.o cpu/op68kadd.o cpu/op68karith.o cpu/op68ksub.o cpu/op68klogop.o \
#		cpu/68000.o

OBJ_CPU_OPHANDLERS = op68kadd.o op68karith.o op68klogop.o op68kmisc.o op68kmove.o op68kshift.o op68ksub.o
OBJ_CPU = 68000.o op68000.o $(OBJ_CPU_OPHANDLERS)
OBJ_MCH = init.o st.o mem.o ikbd.o blitter.o fdc.o
OBJ_APP = main.o atari.o amiga.o atari_start.o atari_asm.o

OBJSNOPATH = $(OBJ_CPU) $(OBJ_MCH) $(OBJ_APP)
OBJS = $(OBJSNOPATH:%=obj/%)

LINKOBJS = \
		obj/op68000.o \
		obj/main.o \
		obj/st.o \
		obj/op68kmove.o \
		obj/op68ksub.o \
		obj/op68kadd.o \
		obj/op68kmisc.o \
		obj/op68kshift.o \
		obj/op68klogop.o \
		obj/68000.o \
		obj/op68karith.o \
		obj/atari.o \
		obj/atari_start.o \
		obj/atari_asm.o \
		obj/amiga.o \
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
	$(CC) $(CFLAGS) -o $@ $(LINKOBJS) $(LFLAGS)
	tools/stripx -v -f -s $(APPLICATION)

obj/op68k%.o : cpu/op68k%.c $(GLOBALDEPENDS) tools
	$(GEN) $< $(@:%.o=%_expanded.c)
	$(CC) $(CFLAGS) -S $(@:%.o=%_expanded.c) -o $(@:%.o=%.0.s)
	$(GEN) $(@:%.o=%.0.s) $(@:%.o=%.1.s)
	$(CC) $(CFLAGS) -c $(@:%.o=%.1.s) -o $@

obj/op68000.o : cpu/op68000.c $(OBJ_CPU_OPHANDLERS:%=obj/%) $(GLOBALDEPENDS) tools
	$(GEN) $< $(@:%.o=%_expanded.c)
	$(CC) $(CFLAGS) -c $(@:%.o=%_expanded.c) -o $@

obj/68000.o : cpu/68000.c obj/op68000.o $(GLOBALDEPENDS)
	$(CC) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : st/%.c $(GLOBALDEPENDS)
	$(CC) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : %.c $(GLOBALDEPENDS)
	$(CC) $(CFLAGS) -S $< -o $(@:%.o=%.s)
	$(CC) $(CFLAGS) -c $(@:%.o=%.s) -o $@

obj/%.o : %.S $(GLOBALDEPENDS)
	$(CC) $(CFLAGS) -c $< -o $@

linkonly:
	$(CC) $(CFLAGS) -o $(APPLICATION) $(LINKOBJS) $(LFLAGS)
	tools/stripx -v -f -s $(APPLICATION)
