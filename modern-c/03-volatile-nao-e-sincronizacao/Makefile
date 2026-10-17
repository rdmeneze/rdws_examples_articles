# Exemplos do artigo "volatile não é sincronização: usando atomics do C11 em firmware".
# host/: roda no Linux, com um sinal SIGALRM fazendo o papel da ISR.
# arm/:  só compila para Cortex-M; o scripts/teste.sh confere o assembly gerado.

CFLAGS ?= -O2
WARN   := -Wall -Wextra
BUILD  := build

PROGS := $(BUILD)/rmw $(BUILD)/torn_read $(BUILD)/ring $(BUILD)/neves
NEVES := $(BUILD)/neves-src

.PHONY: all run teste clean

all: $(PROGS)

$(BUILD)/%: host/%.c host/isr.h host/ringbuf.h
	@mkdir -p $(BUILD)
	$(CC) -std=c11 $(WARN) $(CFLAGS) $< -o $@

run: all
	$(BUILD)/rmw volatile
	$(BUILD)/rmw atomic
	$(BUILD)/torn_read ingenua
	$(BUILD)/torn_read repete
	$(BUILD)/torn_read secao
	$(BUILD)/ring
	$(BUILD)/neves

$(NEVES)/buffer_circ.c:
	scripts/baixa_neves.sh $(NEVES)

$(BUILD)/neves: host/neves.c host/isr.h $(NEVES)/buffer_circ.c
	@mkdir -p $(BUILD)
	$(CC) -std=c11 $(WARN) $(CFLAGS) -I$(NEVES) $< $(NEVES)/buffer_circ.c -o $@

teste:
	scripts/teste.sh

clean:
	rm -rf $(BUILD)
