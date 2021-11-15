
EE_ELF = patch-11184.elf
EE_BIN = patch-11184.bin
EE_OBJS = main.o spectate.o maploader.o rpc.o config.o functions.o
EE_LIBS = -ldl

EE_INCS := -I../common -I../libdl/include
EE_LDFLAGS = -fno-builtin -nostdlib -nostartfiles -L. -L../libdl/lib
#EE_DEFS = -DDEBUG
#EE_DEFS := -DBETA

all: $(EE_BIN) clean copy

clean:
	rm -f *.irx *.o

include ${CURDIR}/../Makefile.pref
include ${CURDIR}/../Makefile.eeglobal
include ${CURDIR}/../Makefile.rules
