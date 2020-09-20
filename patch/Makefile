
EE_ELF = patch-11184.elf
EE_BIN = patch-11184.bin
EE_OBJS = main.o spectate.o
EE_LIBS = -ldl 

EE_INCS := -I../common -I../libdl/include
EE_LDFLAGS = -fno-builtin -nostartfiles -L. -L../libdl/lib

all: $(EE_BIN) clean

clean:
	rm -f *.irx *.o

include $(PWD)/../Makefile.pref
include $(PWD)/../Makefile.eeglobal
