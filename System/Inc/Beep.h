#ifndef __BEEP_H__
#define __BEEP_H__

#include <stdint.h>

#define BEE P32

void BeepSetFreq(uint16_t freq);
void BeepInit(void);

#endif