#include <stdio.h>
#include <gmp.h>

void calc_pn(mpz_t pn)
{
    mpz_init(pn);

    mpz_t a, b, c, t1;
    mpz_init(a);
    mpz_set_ui(a, 2);
    mpz_init(b);
    mpz_set_ui(b, 1);
    mpz_init(c);
    mpz_set_ui(c, 0);
    mpz_init(t1);

    for (int i = 0; i < 199931; i++)
    {
        mpz_set(t1, a);
        mpz_addmul_ui(a, b, 2);
        mpz_addmul_ui(a, c, 3);
        mpz_set(c, b);
        mpz_set(b, t1);
    }

    mpz_set(pn, c);
}