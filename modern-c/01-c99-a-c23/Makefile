# Exemplos do artigo "25 anos de evolução da linguagem C: de C99 a C23".
# Cada pasta é compilada com o padrão correspondente.

CFLAGS ?= -O2
WARN   := -Wall -Wextra -pedantic
BUILD  := build

C99_BIN := $(patsubst %.c,$(BUILD)/%,$(wildcard c99/*.c))
C11_BIN := $(patsubst %.c,$(BUILD)/%,$(wildcard c11/*.c))
C23_BIN := $(patsubst %.c,$(BUILD)/%,$(wildcard c23/*.c))

# c2x é o nome provisório, aceito também por versões mais antigas; GCC 14+ e Clang 18+ aceitam c23.
STD_C23 ?= c2x

# memset_explicit depende da libc, sem macro para testar no código: o Makefile testa.
HAVE_MEMSET_EXPLICIT := $(shell printf '\043include <string.h>\nint main(void){char b[1];memset_explicit(b,0,1);return b[0];}\n' \
	| $(CC) -std=$(STD_C23) -x c - -o /dev/null 2>/dev/null && echo -DHAVE_MEMSET_EXPLICIT)

.PHONY: all c99 c11 c23 run teste clean

all: c99 c11 c23

c99: $(C99_BIN)
c11: $(C11_BIN)
c23: $(C23_BIN)

$(BUILD)/c99/%: c99/%.c
	@mkdir -p $(dir $@)
	$(CC) -std=c99 $(WARN) $(CFLAGS) $< -o $@ -lm

$(BUILD)/c11/%: c11/%.c
	@mkdir -p $(dir $@)
	$(CC) -std=c11 $(WARN) $(CFLAGS) $< -o $@ -lm

$(BUILD)/c23/%: c23/%.c
	@mkdir -p $(dir $@)
	$(CC) -std=$(STD_C23) $(WARN) $(CFLAGS) $(HAVE_MEMSET_EXPLICIT) $< -o $@ -lm

run: all
	@for b in $(C99_BIN) $(C11_BIN) $(C23_BIN); do echo "== $$b"; $$b || exit 1; done

teste:
	scripts/teste.sh

clean:
	rm -rf $(BUILD)
