# Exemplos: quanto do C23 já pode ser usado em sistemas embarcados?

Acompanha o artigo [Quanto do C23 já pode ser usado em sistemas embarcados?](https://embarcados.com.br/quanto-do-c23-ja-pode-ser-usado-em-sistemas-embarcados/),
publicado no Embarcados. Mede, recurso por recurso, o que cada toolchain aceita.

| Pasta | O que tem |
|---|---|
| `testes/` | Um arquivo por recurso do C23. A primeira linha diz o tipo (linguagem, cabeçalho do compilador ou biblioteca) e o que verificar: compila, linka ou roda |
| `scripts/matriz.sh` | Testa todos os recursos em um compilador e imprime uma linha por recurso |
| `scripts/rodar_tudo.sh` | Roda a matriz em todos os toolchains e gera `resultados/matriz.xlsx` |
| `scripts/tabela.py` | Junta os resultados em uma planilha: aba Matriz (cores por resultado e total de "ok" por toolchain) e aba Legenda (significados e método) |
| `compat/` | `compat_c23.h`: usa o recurso do C23 quando existe e um substituto quando não existe (`nullptr`, `bool`, `static_assert`, `ckd_add`, `memset_explicit`); `exemplo.c` usa o cabeçalho; `overflow.c` é o exemplo da abertura do artigo (`__builtin_add_overflow` e `ckd_add`) |
| `scripts/teste_compat.sh` | Compila e roda `compat/exemplo.c` com cada padrão de C11 a C23 que o compilador aceitar |
| `scripts/figura.py` | Gera `resultados/matriz.png`, a figura do artigo: só os recursos em que os toolchains diferem, com colunas vizinhas idênticas agrupadas (precisa do `matplotlib`) |
| `resultados/` | A última matriz gerada (outubro de 2026): `matriz.xlsx` e os `.tsv` de cada toolchain |

## Como os testes são feitos

- Compilação com `-std=c23` (ou `-std=c2x`, se o compilador não aceitar `c23`),
  `-pedantic-errors` e `-Werror`: uma extensão do compilador não conta como suporte ao padrão.
- Recursos de biblioteca também são linkados. Se não compilarem no modo estrito, o script
  tenta de novo com `-std=gnu23`: quando passa, a função existe, mas a libc só a expõe com as
  extensões GNU ("só gnu").
- Nos alvos ARM não há como rodar o binário, então o teste de `printf("%b")` fica como "n/t".

Resultados: `ok`, `não` (não compila), `só declara` (compila, mas não linka), `só gnu`,
`falha` (roda com saída errada) e `n/t` (não testado).

## Rodando

Precisa do `openpyxl` para gerar a planilha (`pip install openpyxl`, ou o pacote `python3-openpyxl` da distribuição) e de Docker para os toolchains de host (imagens `gcc:13` a `gcc:16`,
`silkeh/clang:18` a `22` e `fedora:44`, que traz a glibc 2.43; nela o GCC é instalado com
`dnf` na hora). Os toolchains ARM são opcionais:

```sh
# a figura do artigo, a partir dos .tsv já gerados
scripts/figura.py resultados/matriz.png resultados/*.tsv

# só os toolchains de host
scripts/rodar_tudo.sh

# com as Arm GNU Toolchains (diretórios de instalação) e o Zephyr SDK
ARM_GNU="$HOME/arm/13.3.rel1 $HOME/arm/14.3.rel1 $HOME/arm/15.2.rel1" \
ZEPHYR_SDK="$HOME/zephyr-sdk-1.0.1" \
scripts/rodar_tudo.sh

# um compilador qualquer, direto
scripts/matriz.sh "meu gcc" gcc
ALVO=cross LINK="--specs=nosys.specs" scripts/matriz.sh "arm" arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb
```

As Arm GNU Toolchains são baixadas de
<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads> (pacote
`x86_64-arm-none-eabi`); confira o `.sha256asc` publicado ao lado de cada arquivo.

## Cabeçalho de compatibilidade

```sh
scripts/teste_compat.sh                 # cc do sistema
CC=clang scripts/teste_compat.sh
```

Testado com GCC 13, GCC 15, Clang 18 e Fedora 44 (com `-DHAVE_MEMSET_EXPLICIT`, já que a
glibc 2.43 tem a função), e compilado para Cortex-M0 com a Arm GNU Toolchain 13.3 e o Zephyr
SDK 1.0.1, de `-std=c11` a `-std=c2x`.

## Cuidados ao ler a matriz

- Nas colunas de host, a libc é a da imagem Docker, não a do compilador: as imagens do Clang
  18 a 20 trazem a glibc 2.36, por isso `stdbit.h` aparece como "não" nelas.
- Nos alvos ARM, o binário é linkado com `--specs=nosys.specs` (newlib) ou com a picolibc
  padrão do Zephyr SDK, sem rodar.
