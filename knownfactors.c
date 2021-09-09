#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

mpz_t factorCache[24]; // overly optimistic?
int numfactors = 0;

int loadKnownFactors(mpz_t **factors, int verbose)
{
    if (numfactors == 0)
    {
        FILE *ff = fopen("knownfactors.txt", "r");

        char *line = NULL;
        size_t len = 0;

        if (ff == NULL)
            exit(EXIT_FAILURE);

        while (getline(&line, &len, ff) > 1)
        {
            //printf("%d\n", read);
            numfactors++;
        }

        if (line)
            free(line);

        if (verbose)
        {
            printf("Number of known factors: %d\n", numfactors);
        }

        fseek(ff, 0, 0);

        for (int i = 0; i < numfactors; i++)
        {
            mpz_init(factorCache[i]);
            mpz_inp_str(factorCache[i], ff, 10);
            if (verbose)
            {
                printf("Read factor ");
                mpz_out_str(stdout, 10, factorCache[i]);
                printf("\n");
            }
        }

        fclose(ff);
    }

    *factors = factorCache;
    return numfactors;
}
