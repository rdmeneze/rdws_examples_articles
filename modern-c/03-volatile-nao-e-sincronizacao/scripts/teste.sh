#!/usr/bin/env bash
# Testa os exemplos. No host, roda os programas e confere os números (as falhas do volatile
# são estatísticas: o teste repete até três vezes antes de reclamar). No ARM, se houver um
# GCC para ARM, confere o assembly e o link de cada caso.
#
# Uso: scripts/teste.sh            (CC=clang scripts/teste.sh)
#      ARM_CC=/caminho/arm-none-eabi-gcc scripts/teste.sh
set -uo pipefail

cd "$(dirname "$0")/.."
CC="${CC:-cc}"
falhas=0
ok()    { echo "  ok: $1"; }
falha() { echo "  FALHOU: $1"; falhas=$((falhas + 1)); }

# numero <texto>: o último número antes de um "vezes" ou o último número da linha
perdidos() { grep -o 'perdidos: [0-9]*' <<< "$1" | grep -o '[0-9]*$'; }
recuos()   { grep -o 'trás [0-9]*' <<< "$1" | grep -o '[0-9]*$'; }

echo "== host (um sinal faz o papel da ISR)"
if ! make -s clean || ! make -s CC="$CC" > /dev/null; then
    falha "compilar host/"
else
    for tentativa in 1 2 3; do
        r="$(build/rmw volatile)"; [ "$(perdidos "$r")" -gt 0 ] && break
    done
    echo "  $r"
    # No x86, alguns compiladores (o Clang, por exemplo) fazem o incremento de um volatile numa
    # instrução só (incl mem), que uma interrupção não divide. No Cortex-M isso não existe.
    uma_instrucao="$(printf 'volatile unsigned c;\nvoid f(void){ c++; }\n' | "$CC" -O2 -S -x c - -o - | grep -cE '^\s+(incl|addl)\s.*\(%rip\)')"
    if [ "$(perdidos "$r")" -gt 0 ]; then
        ok "volatile perde incrementos"
    elif [ "$uma_instrucao" -gt 0 ]; then
        ok "não se aplica: este compilador incrementa o volatile numa instrução só (incl), que a ISR não divide"
    else
        falha "volatile não perdeu nada em 3 tentativas"
    fi
    r="$(build/rmw atomic)"; echo "  $r"
    [ "$(perdidos "$r")" -eq 0 ] && ok "atomic não perde nada" || falha "atomic perdeu incrementos"

    for tentativa in 1 2 3; do
        r="$(build/rasgada ingenua)"; [ "$(recuos "$r")" -gt 0 ] && break
    done
    echo "  $r"
    [ "$(recuos "$r")" -gt 0 ] && ok "leitura ingênua rasga" || falha "leitura ingênua não rasgou em 3 tentativas"
    for modo in repete secao; do
        r="$(build/rasgada $modo)"; echo "  $r"
        [ "$(recuos "$r")" -eq 0 ] && ok "leitura '$modo' não rasga" || falha "leitura '$modo' rasgou"
    done

    r="$(build/ring)"; echo "  $r"
    grep -q ' 0 fora de ordem' <<< "$r" && ok "ring buffer entrega tudo em ordem" || falha "ring buffer fora de ordem"

    r="$(build/neves 100000)"; echo "  $r"
    if grep -q ' 0 fora de ordem' <<< "$r"; then
        ok "buffer do Neves: nenhuma falha visível (a corrida existe, mas a janela é de uma instrução)"
    else
        ok "buffer do Neves: a corrida apareceu nesta execução"
    fi
fi

# reordena <compilador e flags...>: quantos STR/MOV de memória sobram em publica() e se a
# escrita de dado ficou depois da de pronto.
reordena() {
    "$@" -std=c11 -O2 -S arm/reordena.c -o - | awk '
        /^publica:/ { f = 1; next }
        f && /^\t(bx|ret)/ { exit }
        f && /(str|mov)/ && /pronto/ { p = NR } 
        f && /(str|mov)/ && /dado/ { d = NR; nd++ }
        f && /^\tstr/ { ns++ }
        END { print (ns ? ns : nd + 1) " " (p ? p : 0) " " (d ? d : 0) }'
}

echo "== reordenação (arm/reordena.c, no host)"
read -r n p d <<< "$(reordena "$CC")"
if [ "$p" -gt 0 ] && [ "$d" -gt "$p" ]; then
    ok "a única escrita em dado ficou depois de pronto = 1"
else
    echo "  (n=$n pronto=$p dado=$d)"; falha "padrão de reordenação não encontrado"
fi

echo "== Cortex-M"
if [ -z "${ARM_CC:-}" ]; then
    ARM_CC="$(command -v arm-none-eabi-gcc || true)"
fi
if [ -z "$ARM_CC" ]; then
    ARM_CC="$(ls ~/zephyr-sdk-*/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc \
                  ~/zephyr-sdk-*/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc 2>/dev/null | head -1 || true)"
fi
if [ -z "$ARM_CC" ]; then
    echo "  pulado: nenhum GCC para ARM (defina ARM_CC)"
else
    echo "  usando $("$ARM_CC" --version | head -1)"
    asm() { "$ARM_CC" -std=c11 -O2 -mcpu="$1" -mthumb -S "$2" -o - ; }
    corpo() { awk -v f="^$1:" '$0 ~ f { d = 1; next } d && /^\t(bx|pop)/ { print; exit } d && /^\t[a-z]/ { print $1 }'; }

    for cpu in cortex-m0 cortex-m4; do
        r="$(asm $cpu arm/rmw.c | corpo liga_bit | tr '\n' ' ')"
        if grep -q 'ldr .*orr.* str' <<< "$r"; then ok "$cpu: flags |= BIT vira ldr/orr/str"; else echo "  $r"; falha "$cpu: rmw"; fi
    done

    neves="build/neves-src"
    if scripts/baixa_neves.sh "$neves" > /dev/null; then
        r="$("$ARM_CC" -std=c11 -O2 -mcpu=cortex-m4 -mthumb -S "$neves/buffer_circ.c" -o - | corpo buffer_retrieve | tr '\n' ' ')"
        if grep -qE 'ldr .*subs? .*str' <<< "$r"; then
            ok "cortex-m4: items-- do buffer do Neves vira ldr/sub/str"
        else
            echo "  $r"; falha "cortex-m4: items-- do buffer do Neves"
        fi
    else
        falha "baixar o buffer do Neves"
    fi

    r="$(asm cortex-m0 arm/ticks64.c | corpo le_ticks | grep -c '^ldr$')"
    [ "$r" -ge 3 ] && ok "cortex-m0: volatile uint64_t lido com dois LDR" || falha "cortex-m0: leitura de 64 bits ($r ldr)"
    r="$(asm cortex-m4 arm/ticks64.c)"
    grep -q 'ldrd' <<< "$r" && grep -q '__atomic_load_8' <<< "$r" \
        && ok "cortex-m4: LDRD no volatile, mas atomic_load de 64 bits vira __atomic_load_8" \
        || falha "cortex-m4: leitura de 64 bits"

    read -r n p d <<< "$(reordena "$ARM_CC" -mcpu=cortex-m4 -mthumb)"
    [ "$n" -eq 2 ] && ok "cortex-m4: publica() com só duas escritas (a primeira em dado sumiu)" || falha "cortex-m4: reordenação ($n escritas)"

    esperado="st_relaxed:0 st_release:1 st_seq_cst:2 ld_relaxed:0 ld_acquire:1 ld_seq_cst:2"
    r="$(asm cortex-m4 arm/ordens.c | awk '/^(st|ld)_[a-z_]+:/ { f = $1; sub(":", "", f); n[f] = 0; o[++k] = f; next }
            f && /^\tdmb/ { n[f]++ } /^\tbx/ { f = "" }
            END { for (i = 1; i <= k; i++) printf "%s:%d ", o[i], n[o[i]] }')"
    [ "${r% }" = "$esperado" ] && ok "cortex-m4: DMBs por ordem de memória: $esperado" || { echo "  $r"; falha "ordens de memória"; }

    if "$ARM_CC" -std=c11 -O2 -mcpu=cortex-m4 -mthumb arm/fetch_add.c -o /dev/null 2>/dev/null; then
        ok "cortex-m4: atomic_fetch_add linka"
    else
        falha "cortex-m4: atomic_fetch_add não linkou"
    fi
    r="$("$ARM_CC" -std=c11 -O2 -mcpu=cortex-m0 -mthumb arm/fetch_add.c -o /dev/null 2>&1)"
    grep -q "undefined reference to \`__atomic_fetch_add_4'" <<< "$r" \
        && ok "cortex-m0: atomic_fetch_add falha no link (__atomic_fetch_add_4)" \
        || falha "cortex-m0: esperava erro de link"

    tmp="$(mktemp -d)"
    if "$ARM_CC" -std=c11 -O2 -Wall -mcpu=cortex-m0 -mthumb -c arm/ring.c -o "$tmp/ring.o" \
       && ! "$ARM_CC" -mcpu=cortex-m0 -mthumb -nostdlib -Wl,--entry=0 "$tmp/ring.o" -o /dev/null 2>&1 | grep -q '__atomic'; then
        ok "cortex-m0: ring buffer linka sem nenhuma __atomic_* ($(asm cortex-m0 arm/ring.c | grep -c dmb) DMBs)"
    else
        falha "cortex-m0: ring buffer"
    fi
    rm -rf "$tmp"

    r="$(asm cortex-m0 arm/secao_critica.c | corpo le_ticks | tr '\n' ' ')"
    grep -q 'mrs.*cpsid.*msr' <<< "$r" && ok "cortex-m0: seção crítica com mrs/cpsid ... msr" || { echo "  $r"; falha "seção crítica"; }
fi

make -s clean
echo
if [ "$falhas" -eq 0 ]; then echo "tudo certo"; else echo "$falhas falha(s)"; exit 1; fi
