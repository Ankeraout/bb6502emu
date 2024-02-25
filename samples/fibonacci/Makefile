MKDIR := mkdir
CA65 := ca65
LD65 := ld65
RM := rm

all: bin/fibonacci.bin

bin/fibonacci.bin: obj/fibonacci.o
	@echo LD65 $@
	@if [ ! -d bin ]; then \
		$(MKDIR) bin; \
	fi
	@$(LD65) -C src/linker.cfg $^ -o $@

obj/%.o: src/%.s
	@echo CA65 $@
	@if [ ! -d obj ]; then \
		$(MKDIR) obj; \
	fi
	@$(CA65) $< -o $@

clean:
	@echo RM obj bin
	@$(RM) -rf obj bin

.PHONY: all clean
