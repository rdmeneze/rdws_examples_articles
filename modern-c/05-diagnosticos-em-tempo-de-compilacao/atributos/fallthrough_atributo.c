/* fallthrough: atributo */
void a(void);
void b(void);

void f(int x)
{
    switch (x) {
    case 0:
        a();
        [[fallthrough]];
    case 1:
        b();
        break;
    }
}
