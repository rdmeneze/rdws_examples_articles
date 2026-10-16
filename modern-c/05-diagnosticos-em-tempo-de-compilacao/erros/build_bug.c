/* Erro sob medida com o atributo error do GCC: a função nunca é definida, e a chamada só pode
 * sobrar no código se o otimizador não provar que ela é inalcançável. Depende do -O. */
[[gnu::error("ponto alcançável")]]
void fw_unreachable_ct(void);

static int indice_ok(int v)
{
    return v >= 0 && v < 4;
}

int consulta(void)
{
    int idx = 2;

    if (!indice_ok(idx)) {
        fw_unreachable_ct();   /* em -O2 some; em -O0 sobra e dá erro */
    }
    return idx;
}

#ifdef ALCANCAVEL
int consulta_externa(int v)
{
    if (v > 0) {
        return 1;
    }
    fw_unreachable_ct();       /* alcançável de verdade: erro em qualquer -O */
    return 0;
}
#endif
