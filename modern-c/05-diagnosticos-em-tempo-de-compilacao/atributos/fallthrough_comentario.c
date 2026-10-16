/* fallthrough: comentario */
void a(void);
void b(void);

void f(int x)
{
    switch (x) {
    case 0:
        a();
        /* fall through */
    case 1:
        b();
        break;
    }
}
