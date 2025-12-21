#ifndef __LED_H__
#define __LED_H__

#include "stdint.h"

#define DS P54
#define ST_CP P33
#define SH_CP P30

// 四串74hc595共32位
#define LED_9   (1UL << 6)
#define LED_18  (1UL << 7)
#define LED_1   (1UL << 8)
#define LED_2   (1UL << 9)
#define LED_10  (1UL << 10)
#define LED_19  (1UL << 11)
#define LED_3   (1UL << 12)
#define LED_11  (1UL << 13)
#define LED_20  (1UL << 14)
#define LED_12  (1UL << 15)
#define LED_4   (1UL << 16)
#define LED_21  (1UL << 17)
#define LED_22  (1UL << 18)
#define LED_5   (1UL << 19)
#define LED_13  (1UL << 20)
#define LED_23  (1UL << 21)
#define LED_14  (1UL << 22)
#define LED_6   (1UL << 23)
#define LED_24  (1UL << 24)
#define LED_7   (1UL << 25)
#define LED_25  (1UL << 26)
#define LED_15  (1UL << 27)
#define LED_26  (1UL << 28)
#define LED_16  (1UL << 29)
#define LED_8   (1UL << 30)
#define LED_17  (1UL << 31)

#define LED_A LED_1
#define LED_B LED_2
#define LED_C LED_3
#define LED_D LED_4
#define LED_E LED_5
#define LED_F LED_6
#define LED_G LED_7
#define LED_H LED_8
#define LED_I LED_9
#define LED_J LED_10
#define LED_K LED_11
#define LED_L LED_12
#define LED_M LED_13
#define LED_N LED_14
#define LED_O LED_15
#define LED_P LED_16
#define LED_Q LED_17
#define LED_R LED_18
#define LED_S LED_19
#define LED_T LED_20
#define LED_U LED_21
#define LED_V LED_22
#define LED_W LED_23
#define LED_X LED_24
#define LED_Y LED_25
#define LED_Z LED_26

void Led_Init(void);
void Led_write(uint32_t data, uint8_t len);
void Led_Append(uint32_t data, uint8_t len);
void Led_DisplayChar(char c);
void Led_DisplayString(const char* str, uint16_t delay_ms);

#endif
