#!/usr/bin/env python3
"""Gera a figura do artigo a partir dos .tsv de scripts/matriz.sh: só os recursos em que os
toolchains diferem, com colunas vizinhas de resultados idênticos agrupadas. Precisa do
matplotlib (pip install matplotlib).

Uso: scripts/figura.py resultados/matriz.png resultados/*.tsv
"""
import pathlib
import re
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

TESTES = pathlib.Path(__file__).resolve().parent.parent / "testes"

# Tons claros da paleta de status, com texto escuro: o rótulo carrega o significado.
CORES = {"ok": "#cdebcd", "não": "#f6d3d3", "só gnu": "#fde9c0", "só declara": "#fde9c0", "n/t": "#e6e6e3"}
TINTA, TINTA_SEC, GRADE = "#1f1f1e", "#5c5c58", "#ffffff"

ROTULOS = {
    "04-literais-binarios": "literais binários 0b",
    "07-constexpr": "constexpr",
    "13-embed": "#embed",
    "14-bitint": "_BitInt(N)",
    "17-stdckdint": "stdckdint.h (ckd_add)",
    "18-stdbit": "stdbit.h",
    "19-memset-explicit": "memset_explicit",
    "20-strdup": "strdup / strndup",
    "21-memccpy": "memccpy",
    "22-timegm": "timegm",
    "23-printf-b": "printf(\"%b\")",
}


def le(caminhos):
    colunas, dados = [], {}
    for c in caminhos:
        for linha in pathlib.Path(c).read_text(encoding="utf-8").splitlines():
            nome, rec, res = linha.split("\t")
            if nome not in colunas:
                colunas.append(nome)
            dados[(rec, nome)] = res
    return colunas, dados


def tipo(rec):
    arq = TESTES / f"{rec}.c"
    cab = arq.read_text(encoding="utf-8").splitlines()[0] if arq.exists() else ""
    m = re.search(r"tipo: ([^;]+);", cab)
    return m.group(1).strip() if m else ""


def curto(col, dados):
    comp = dados[("_compilador", col)].replace("Clang ", "Clang ").split(".")
    nome = comp[0] if "GCC" in comp[0] or "Clang" in comp[0] else comp[0]
    libc = dados[("_libc", col)]
    libc = re.sub(r"\.0$", "", libc)
    return nome, libc


def main():
    saida, entradas = sys.argv[1], sys.argv[2:]
    colunas, dados = le(entradas)
    recursos = sorted({r for r, _ in dados if not r.startswith("_")})
    varia = [r for r in recursos if len({dados[(r, c)] for c in colunas}) > 1]

    # Agrupa colunas vizinhas com o mesmo resultado em todos os recursos e a mesma libc.
    grupos = []
    for c in colunas:
        assinatura = (tuple(dados[(r, c)] for r in recursos), curto(c, dados)[1], "arm" in c or "Zephyr" in c)
        if grupos and grupos[-1]["assinatura"] == assinatura:
            grupos[-1]["cols"].append(c)
        else:
            grupos.append({"assinatura": assinatura, "cols": [c]})

    def cabecalho(g):
        nomes = [curto(c, dados)[0] for c in g["cols"]]
        libc = curto(g["cols"][0], dados)[1]
        if "Zephyr" in g["cols"][0]:
            return "Zephyr\nSDK 1.0.1", libc
        if g["cols"][0].startswith("arm-none-eabi"):
            return "Arm\n" + g["cols"][0].split(" ")[1].replace(".rel1", ""), libc
        if g["cols"][0].startswith("fedora"):
            return "Fedora\n44", libc
        if len(nomes) > 1:
            base = nomes[0].split(" ")[0]
            return f"{base} {nomes[0].split(' ')[1]}–{nomes[-1].split(' ')[1]}", libc
        return nomes[0], libc

    n_l, n_c = len(varia), len(grupos)
    larg_rot, larg_col, alt = 3.0, 1.0, 0.5
    fig_w = larg_rot + n_c * larg_col + 0.3
    fig_h = 1.75 + n_l * alt + 1.45
    plt.rcParams["font.family"] = "Liberation Sans"
    # As coordenadas ficam em "unidades de desenho"; a figura física é ESCALA vezes menor,
    # o que deixa o texto legível depois de reduzida à largura de uma página.
    ESCALA = 2.0
    fig = plt.figure(figsize=(fig_w / ESCALA, fig_h / ESCALA), dpi=300)
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(0, fig_w)
    ax.set_ylim(fig_h, 0)
    ax.axis("off")

    x0, y0 = larg_rot, 1.75
    # Faixas de grupo: host x Cortex-M4.
    n_host = sum(1 for g in grupos if not g["assinatura"][2])
    for ini, n, texto in [(0, n_host, "Host x86-64 (Docker)"), (n_host, n_c - n_host, "Cortex-M4 (sem execução)")]:
        if n:
            xa, xb = x0 + ini * larg_col + 0.04, x0 + (ini + n) * larg_col - 0.04
            ax.plot([xa, xb], [0.42, 0.42], color=TINTA_SEC, lw=0.8)
            ax.text((xa + xb) / 2, 0.36, texto, ha="center", va="bottom", fontsize=7.5, color=TINTA_SEC)
    for j, g in enumerate(grupos):
        comp, libc = cabecalho(g)
        cx = x0 + (j + 0.5) * larg_col
        comp = comp if "\n" in comp else comp.replace(" ", "\n", 1)
        ax.text(cx, y0 - 0.5, comp, ha="center", va="bottom", fontsize=7.5, fontweight="bold", color=TINTA, linespacing=1.05)
        ax.text(cx, y0 - 0.06, libc.replace(" ", "\n", 1), ha="center", va="bottom", fontsize=6.8, color=TINTA_SEC, linespacing=1.0)

    # Dois blocos: o que depende do compilador e o que depende da libc.
    def bloco(r):
        return "biblioteca (libc)" if tipo(r) == "biblioteca" else "compilador"
    i = 0
    while i < n_l:
        k = i
        while k < n_l and bloco(varia[k]) == bloco(varia[i]):
            k += 1
        ya, yb = y0 + i * alt + 0.06, y0 + k * alt - 0.06
        ax.plot([0.12, 0.12], [ya, yb], color=TINTA_SEC, lw=1.2)
        ax.text(0.04, (ya + yb) / 2, bloco(varia[i]), rotation=90, ha="center", va="center", fontsize=6.8, color=TINTA_SEC)
        if i:
            ax.plot([0.22, x0 + n_c * larg_col], [y0 + i * alt, y0 + i * alt], color=TINTA_SEC, lw=0.8)
        i = k

    for i, r in enumerate(varia):
        y = y0 + i * alt
        ax.text(0.3, y + alt / 2, ROTULOS.get(r, r), ha="left", va="center", fontsize=7.5, color=TINTA,
                family="DejaVu Sans Mono" if r in ("13-embed", "23-printf-b") else None)
        for j, g in enumerate(grupos):
            res = dados[(r, g["cols"][0])]
            ax.add_patch(Rectangle((x0 + j * larg_col, y), larg_col, alt, facecolor=CORES.get(res, "#ffffff"),
                                   edgecolor=GRADE, linewidth=2))
            ax.text(x0 + (j + 0.5) * larg_col, y + alt / 2, res, ha="center", va="center", fontsize=7.5, color=TINTA)

    y_leg = y0 + n_l * alt + 0.25
    x = 0.15
    for res, txt in [("ok", "ok: funciona"), ("não", "não: não compila"), ("só gnu", "só gnu: só com -std=gnu23"),
                     ("n/t", "n/t: não testado (alvo sem execução)")]:
        ax.add_patch(Rectangle((x, y_leg), 0.28, 0.26, facecolor=CORES[res], edgecolor=TINTA_SEC, linewidth=0.5))
        ax.text(x + 0.38, y_leg + 0.13, txt, ha="left", va="center", fontsize=7, color=TINTA)
        x += 0.65 + 0.115 * len(txt)
    ax.text(0.15, y_leg + 0.88,
            "Recursos do C23 em que os toolchains diferem; os demais testados funcionam em todos.\n"
            "Compilação com -std=c23 (ou c2x), -pedantic-errors e -Werror. Outubro de 2026.\n"
            "Fedora 44: GCC 16. Arm: Arm GNU Toolchain (GCC). Zephyr SDK 1.0.1: GCC 14.3.",
            ha="left", va="center", fontsize=6.5, color=TINTA_SEC, linespacing=1.3)
    fig.savefig(saida, dpi=300, facecolor="white", bbox_inches="tight", pad_inches=0.1)
    print(f"figura: {saida} ({n_l} recursos, {n_c} colunas)")
    for g in grupos:
        print("  coluna:", " + ".join(g["cols"]))


if __name__ == "__main__":
    main()
