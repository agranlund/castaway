# ------------------------------------
# Castaway
# ------------------------------------
CC = gcc
MT = -j24

# ------------------------------------
# build targets
# ------------------------------------
atari: dirs tools
	make -f Makefile_atari.mak $(MT) target="atari"

raven: dirs tools
	make -f Makefile_atari.mak $(MT) target="raven"

vampire: dirs tools
	make -f Makefile_atari.mak $(MT) target="vampire"

firebee: dirs tools
	make -f Makefile_atari.mak $(MT) target="firebee"

fakefirebee: dirs tools
	make -f Makefile_atari.mak $(MT) target="fakefirebee"

testspeed: dirs tools
	make -f Makefile_atari.mak $(MT) target="atari" defs="-DDEBUG_SPEED"

dist: dirs
	make -f Makefile clean
	make -f Makefile atari build=dist


clean:
	rm -f obj/*
	rm -f tools/stripx
	rm -f tools/mkdisk
	rm -f tools/rdsingle
	rm -f tools/jmptable

# ------------------------------------
# tools
# ------------------------------------
tools: tools/mkdisk tools/rdsingle tools/stripx tools/jmptable

tools/stripx: tools/stripx.c
	$(CC) -o tools/stripx tools/stripx.c

tools/mkdisk: tools/mkdisk.c
	$(CC) -o tools/mkdisk tools/mkdisk.c

tools/rdsingle: tools/rdsingle.c
	$(CC) -o tools/rdsingle tools/rdsingle.c

tools/jmptable: tools/jmptable.c
	$(CC) -o tools/jmptable tools/jmptable.c

dirs:
	mkdir -p obj
