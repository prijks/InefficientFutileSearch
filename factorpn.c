#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <gmp.h>

#include "pn.h"
#include "knownfactors.h"

mpz_t pn, sqrtpn, tmp, mod, *factors;

mpz_t currentFactor;
char *progressFileName;
pid_t child_pid;

char *forwardProgressFileName = "ForwardProgress.txt";
char *reverseProgressFileName = "ReverseProgress.txt";

void printCurrentFactor(int addNewline)
{
    if (child_pid == 0)
    {
        printf("sqrt(p4q) - ");
        mpz_sub(tmp, sqrtpn, currentFactor);
        mpz_out_str(stdout, 10, tmp);
    }
    else
    {
        mpz_out_str(stdout, 10, currentFactor);
    }
    if (addNewline)
    {
        printf("\n");
    }
}

void saveProgress()
{
    FILE *pf = fopen(progressFileName, "w");
    mpz_out_str(pf, 10, currentFactor);
    fclose(pf);
}

void sigSaveProgress(int s)
{
    printf("Received term signal. Saving progress...\n");

    printf("%s search made it to ", child_pid == 0 ? "Reverse" : "Forward");
    printCurrentFactor(1);

    saveProgress();

    printf("Exiting...\n");

    if (child_pid > 0)
    {
        kill(child_pid, s);
        sleep(1);
    }
    exit(0);
}

int main()
{
    calc_pn(pn);

    mpz_init(currentFactor);
    mpz_init(sqrtpn);
    mpz_init(mod);

    int numFactors = loadKnownFactors(&factors, 1);

    for (int i = 0; i < numFactors; i++)
    {
        mpz_div(pn, pn, factors[i]);
    }

    // int isp = mpz_probab_prime_p(pn, 1);
    // printf("isprime? %d\n", isp);
    // return 0;

    mpz_sqrt(sqrtpn, pn);

    if ((child_pid = fork()) < 0)
    {
        printf("Failed to fork reverse search process...\n");
        printf("Guess we'll see if we can do the forward search.\n");
    }

    if (child_pid == 0)
    {
        progressFileName = reverseProgressFileName;
    }
    else
    {
        progressFileName = forwardProgressFileName;
    }

    printf("Checking for progress file...\n");
    FILE *pf = fopen(progressFileName, "r");
    if (pf)
    {
        mpz_inp_str(currentFactor, pf, 10);
        printf("Found progress set to ");
        printCurrentFactor(1);
        printf("\nResuming...\n");
    }
    else
    {
        if (child_pid == 0)
        {
            mpz_set(currentFactor, sqrtpn);
            do
            {
                mpz_add_ui(currentFactor, currentFactor, 1);
                mpz_mod_ui(mod, currentFactor, 2);
            } while (mpz_cmp_ui(mod, 0) == 0);
            printf("No progress file found, starting with sqrt\n");
        }
        else
        {
            mpz_set(currentFactor, factors[numFactors - 1]);
            printf("No progress file found, starting with last known factor ");
            printCurrentFactor(1);
        }
        printf("Beginning...");
    }

    printf("Calculating additional factors...\n");

    signal(SIGINT, sigSaveProgress);
    signal(SIGTERM, sigSaveProgress);

    int i = 0;
    int reportinterval = child_pid == 0 ? 20000 : 1000000;
    clock_t begin = clock();

    int addends[] = {2, 2, 2, 4};
    int addend = 0;

    if (child_pid == 0)
    {
        // reverse search, want to start with a digit ending in 3 -> 3-2=1 1-2=9 9-2=7 7-4=3
        mpz_mod_ui(mod, currentFactor, 10);
        while (mpz_cmp_ui(mod, 3) != 0)
        {
            mpz_add_ui(currentFactor, currentFactor, 2);
            mpz_mod_ui(mod, currentFactor, 10);
        }
    }
    else
    {
        // forward search, want to start with a digit ending in 7 -> 7+2=9 9+2=1 1+2=3 3+4=7
        mpz_mod_ui(mod, currentFactor, 10);
        while (mpz_cmp_ui(mod, 7) != 0)
        {
            mpz_sub_ui(currentFactor, currentFactor, 2);
            mpz_mod_ui(mod, currentFactor, 10);
        }
    }

    while (1) // (mpz_cmp(currentFactor, sqrtpn) < 0)
    {
        mpz_mod(mod, pn, currentFactor);
        if (mpz_cmp_ui(mod, 0) == 0)
        {
            printf("Found factor ");
            mpz_out_str(stdout, 10, currentFactor);
            printf("\n");

            FILE *pf = fopen("newestfactor.txt", "w");
            mpz_out_str(pf, 10, currentFactor);
            fclose(pf);

            break;
        }
        if (i == reportinterval)
        {
            i = 0;
            clock_t end = clock();
            printf("Made it to ");
            printCurrentFactor(0);
            double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
            printf(" without finding a factor yet. %f seconds elapsed: %f checks per second\n", seconds, reportinterval / seconds);
            saveProgress();
            begin = clock();
        }
        else
        {
            i++;
        }
        if (child_pid == 0)
        {
            mpz_sub_ui(currentFactor, currentFactor, addends[addend++]);
            if (addend > 3)
            {
                addend = 0;
            }
        }
        else
        {
            mpz_add_ui(currentFactor, currentFactor, addends[addend++]);
            if (addend > 3)
            {
                addend = 0;
            }
        }
    }

    if (child_pid > 0)
    {
        kill(child_pid, SIGTERM);
    }
    else if (child_pid == 0)
    {
        kill(getppid(), SIGTERM);
    }

    return 0;
}
