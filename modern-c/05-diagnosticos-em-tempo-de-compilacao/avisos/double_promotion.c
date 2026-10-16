float celsius(float f)
{
    return (f - 32) / 1.8;        /* 1.8 é double: a divisão vira double */
}
float celsius_f(float f)
{
    return (f - 32) / 1.8f;
}
