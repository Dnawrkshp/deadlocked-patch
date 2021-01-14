
EE_ELF = patch-11184.elf
EE_BIN = patch-11184.bin
EE_OBJS = main.o spectate.o maploader.o rpc.o
EE_LIBS = -ldl -lkernel

EE_INCS := -I../common -I../libdl/include
EE_LDFLAGS = -fno-builtin -nostdlib -nostartfiles -L. -L../libdl/lib -L$(PS2SDK)/ee/lib
EE_DEFS = -DDEBUG

all: $(EE_BIN) clean

clean:
	rm -f *.irx *.o

include ${CURDIR}/../Makefile.pref
include ${CURDIR}/../Makefile.eeglobal
