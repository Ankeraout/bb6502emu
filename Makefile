# Project-specific configuration
PROJECT_NAME := bb6502emu

# Commands
CC := gcc -c
LD := gcc
MKDIR := mkdir
RM := rm

# Compiler configuration
CFLAGS := -W -Wall -Wextra -O0 -g3 -std=gnu99 -pedantic -Iinclude -MP -MMD
LDFLAGS := -W -Wall -Wextra -O0 -g3

# Automatic generation of code
SOURCES_C := $(shell find src -name '*.c')
OBJECTS_C := $(SOURCES_C:src/%.c=obj/%.o)
DIRS := bin obj $(dir $(OBJECTS_C))

EXECUTABLE_NAME := $(PROJECT_NAME).elf

all: bin/$(EXECUTABLE_NAME)

bin/$(EXECUTABLE_NAME): $(OBJECTS_C) dirs
	$(LD) $(OBJECTS_C) $(LDFLAGS) -o $@

obj/%.o: src/%.c dirs
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf bin obj

dirs:
	$(MKDIR) -p $(DIRS)

.PHONY: all clean dirs
