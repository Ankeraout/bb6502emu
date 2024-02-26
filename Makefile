# Project-specific configuration
PROJECT_NAME := bb6502emu

# Commands
CC := gcc
LD := gcc
MKDIR := mkdir -p
RM := rm
ECHO := echo

# Compiler configuration
CFLAGS := -W -Wall -Wextra -O0 -g3 -std=gnu17 -pedantic -Iinclude -MP -MMD -c
LDFLAGS := -W -Wall -Wextra -O0 -g3

# Automatic generation of code
SOURCES_C := $(shell find src -name '*.c')
OBJECTS_C := $(SOURCES_C:src/%.c=obj/%.o)
DIRS := bin obj $(dir $(OBJECTS_C))

EXECUTABLE_NAME := $(PROJECT_NAME).elf

all: bin/$(EXECUTABLE_NAME)

bin/$(EXECUTABLE_NAME): $(OBJECTS_C)
	@if [ ! -d bin ]; then \
		$(ECHO) MKDIR bin; \
		$(MKDIR) bin; \
	fi
	@$(ECHO) LD $@
	@$(LD) $(OBJECTS_C) $(LDFLAGS) -o $@

obj/%.o: src/%.c
	@if [ ! -d $(dir $@) ]; then \
		$(ECHO) MKDIR $(dir $@); \
		$(MKDIR) $(dir $@); \
	fi
	@$(ECHO) CC $@
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@$(ECHO) RM bin obj
	@$(RM) -rf bin obj

.PHONY: all clean

-include $(OBJECTS_C:%.o=%.d)
