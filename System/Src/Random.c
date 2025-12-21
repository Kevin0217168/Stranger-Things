#include "Random.h"

static uint32_t seed = 0;

void RandomSeedInit(uint32_t initial_seed)
{
    seed = initial_seed;
}

uint32_t RandomGet(void)
{
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF; // Linear congruential generator
    return seed;
}

uint32_t RandomGetRange(uint32_t min, uint32_t max)
{
    if (max <= min) return min;
    return (RandomGet() % (max - min + 1)) + min;
}