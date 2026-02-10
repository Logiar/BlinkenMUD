NAME    = BlinkenMUD
CC      = gcc
PROF    = -O -ggdb
BASE_C_FLAGS =  $(PROF) $(NOCRYPT) -std=gnu99 -Wall -Wformat -Wformat-security -Wunused-value \
 -Wextra -Wno-unused-parameter -fcommon 
L_FLAGS =  $(PROF)
LDLIBS  = -lcrypt

# Prefer the newest GNU C standard supported by the current compiler.
# Use recursive expansion so this probe runs only when C_STD/C_FLAGS are needed.
C_STD = $(shell if $(CC) -std=gnu23 -x c -c /dev/null -o /dev/null >/dev/null 2>&1; then echo gnu23; elif $(CC) -std=gnu2x -x c -c /dev/null -o /dev/null >/dev/null 2>&1; then echo gnu2x; else echo gnu99; fi)
C_FLAGS = $(filter-out -std=%,$(BASE_C_FLAGS)) -std=$(C_STD)

BMUD_FILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
          alias.o ban.o clans.o comm.o const.o curse.o db.o db2.o effects.o \
	  fight.o finger.o flags.o forget.o handler.o healer.o interp.o note.o \
	  lookup.o magic.o magic2.o music.o recycle.o repent.o save.o scan.o \
	  sign.o skills.o special.o tables.o update.o wizlist.o \
          mob_cmds.o mob_prog.o olc.o olc_act.o olc_save.o bit.o mem.o \
          string.o olc_mpcode.o minimap.o bank.o reward.o remort.o quest.o  \
	  teleport.o wpeace.o throwing.o pathfind.o sharpen.o



.PHONY: all rot clean test

all:: rot

rot: $(BMUD_FILES)
	rm -f $(NAME)
	$(CC) $(L_FLAGS) -o $(NAME) $(BMUD_FILES) $(LDLIBS)
	@mkdir -p ../bin
	@mv ./$(NAME) ../bin/

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean: 
	@rm -fr *.o
	@rm -fr ../bin/$(NAME)
	@echo "BlinkenMUD has been cleaned"

test:
	@$(MAKE) clean
	@$(MAKE) rot
	@echo "Build test passed ($(C_STD))"
