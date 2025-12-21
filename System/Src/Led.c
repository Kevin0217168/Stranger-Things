#include "Led.h"
#include "Delay.h"
#include "Beep.h"
#include "stc8_sdcc.h"
#include "stdint.h"

__code uint32_t LedData[] = {LED_A, LED_B, LED_C, LED_D, LED_E, LED_F,
                             LED_G, LED_H, LED_I, LED_J, LED_K, LED_L,
                             LED_M, LED_N, LED_O, LED_P, LED_Q, LED_R,
                             LED_S, LED_T, LED_U, LED_V, LED_W, LED_X,
                             LED_Y, LED_Z};

uint32_t Led_DisplayData = 0UL;

void Led_Init(void)
{
    DS = 0;
    SH_CP = 0;
    ST_CP = 0;
    Led_write(0UL, 32);
}

void Led_write(uint32_t data, uint8_t len)
{
    // 先定义掩码，每次只移一位，速度更快
    uint32_t hide_code = 1UL;
    for (int bit_index = 0; bit_index < len; bit_index++)
    {
        SH_CP = 0;
        DS = (data & hide_code) ? 1 : 0; // 强制 0/1，避免位赋值被截断
        SH_CP = 1;
        hide_code = hide_code << 1;
    }
    DS = 0;
    SH_CP = 0;
    __asm__("nop");
    ST_CP = 1;
    __asm__("nop");
    ST_CP = 0;

    Led_DisplayData = data;
}

void Led_Append(uint32_t data, uint8_t len)
{
    // 先把 32 位结果计算到临时变量，避免编译器在函数调用时传参出错
    uint32_t combined = Led_DisplayData | data;
    Led_write(combined, len);
}

void Led_DisplayChar(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        Led_write(LedData[c - 'A'], 32);
    }
    else
    {
        return;
    }
}

void Led_DisplayString(const char* str, uint16_t ms)
{
    while (*str)
    {
        if (*str >= 'A' && *str <= 'Z')
        {
            Led_DisplayChar(*str);
        }else
        {
            Led_write(0UL, 32); // 清屏
        }
        str++;
        BeepPlay(220, 100);
        delay_ms(ms-100);
    }
}

