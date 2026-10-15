# Exemplos do artigo "Extensões do GCC que facilitam a vida no firmware".
# Tudo em -std=gnu17, o modo que habilita as extensões sem a grafia reservada.

CFLAGS ?= -O2
WARN   := -Wall -Wextra
STD    := -std=gnu17
BUILD  := build

PROGS := $(patsubst %.c,$(BUILD)/%,$(wildcard atributos/*.c builtins/*.c sintaxe/*.c))
PROGS += $(BUILD)/atributos/weak/vetores $(BUILD)/atributos/weak/vetores_uart
PROGS += $(BUILD)/compat/exemplo $(BUILD)/compat/exemplo_outro

.PHONY: all run asm teste clean

all: $(PROGS)

$(BUILD)/%: %.c
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(WARN) $(CFLAGS) $< -o $@

$(BUILD)/atributos/weak/vetores: atributos/weak/vetores.c
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(WARN) $(CFLAGS) $^ -o $@

$(BUILD)/atributos/weak/vetores_uart: atributos/weak/vetores.c atributos/weak/uart.c
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(WARN) $(CFLAGS) $^ -o $@

$(BUILD)/compat/exemplo_outro: compat/exemplo.c compat/compiler.h
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(WARN) $(CFLAGS) -DCOMPAT_SIMULA_OUTRO_COMPILADOR $< -o $@

asm: $(BUILD)/asm/barreira.s

$(BUILD)/asm/barreira.s: asm/barreira.c
	@mkdir -p $(dir $@)
	$(CC) $(STD) -O2 -S $< -o $@

run: all
	@for b in $(PROGS); do echo "== $$b"; $$b || exit 1; done

teste:
	scripts/teste.sh

clean:
	rm -rf $(BUILD)
