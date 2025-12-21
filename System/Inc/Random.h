#ifndef __RANDOM_H_
#define __RANDOM_H_

#include "stdint.h"

void RandomSeedInit(uint32_t initial_seed);
uint32_t RandomGet(void);
uint32_t RandomGetRange(uint32_t min, uint32_t max);

#endif