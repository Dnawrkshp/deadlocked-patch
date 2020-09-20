EE_SRC_DIR = src/
EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/

EE_INCS += -I./include -I$(EE_SRC_DIR)

EE_LIB = $(EE_LIB_DIR)libdl.a
EE_OBJS = libc.o math.o pad.o cheats.o moby.o game.o gamesettings.o \
			sha1.o weapon.o hud.o player.o team.o vector.o

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

$(EE_OBJS_DIR):
	mkdir -p $(EE_OBJS_DIR)

$(EE_LIB_DIR):
	mkdir -p $(EE_LIB_DIR)

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.c
	$(EE_C_COMPILE) -c $< -o $@

install: all
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/include
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f $(EE_LIB) $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f include/*.h $(DESTDIR)$(PS2SDK)/ports/include

clean:
	rm -f -r $(EE_OBJS_DIR)

sample:

include $(PS2SDK)/samples/Makefile.pref
include Makefile.eeglobal
