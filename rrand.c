#include "currency.h"

#ifndef SRAND_SEED
#define SRAND_SEED "/dev/random"
#endif

int32_t rrand(int32_t limit)
{

    FILE *fs_p = NULL;
    uint32_t seed = 0;
    int32_t temp;
    fs_p = fopen(SRAND_SEED, "r");
    if (NULL == fs_p)
    {
        errnum = ERR_SRAND_FLASE;
        return -1;
    }
    else
    {
        fread(&seed, sizeof(int), 1, fs_p); //obtain one unsigned int data
        fclose(fs_p);
    }

    srand(seed);
    temp = (int32_t)rand() % limit;
    return temp;
}