
EE_ELF = race-11184.elf
EE_BIN = race-11184.bin
EE_OBJS = main.o
EE_LIBS = -ldl

EE_INCS := -I../common -I../libdl/include
EE_LDFLAGS = -fno-builtin -nostdlib -nostartfiles -L. -L../libdl/lib
#EE_DEFS = -DDEBUG

all: $(EE_BIN) clean

clean:
	rm -f *.irx *.o

include $(CURDIR)/../Makefile.pref
include $(CURDIR)/../Makefile.eeglobal
