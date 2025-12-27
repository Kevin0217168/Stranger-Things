#ifndef __BEEP_H__
#define __BEEP_H__

#include <stdint.h>

#define BEE P32

typedef enum
{
    BEEP_OFF,
    BEEP_ON,
} BeepState;

typedef struct
{
    uint32_t end_tick;
    BeepState state;
} BeepTask;

void BeepSetFreq(uint16_t freq);
void BeepInit(void);
void BeepPlay(uint16_t freq, uint16_t duration_tick);
void BeepProcess(BeepTask* beepTask);

#endif