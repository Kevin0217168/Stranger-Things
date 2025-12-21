#include "stc8_sdcc.h"
#include "stdint.h"

#define DEBUG_ENABLE 1

#include "Serial.h"
#include "Beep.h"
#include "Delay.h"
#include "Music.h"
#include "Debug.h"
#include "Music_source.h" // 包含音乐数据
#include "Led.h"
#include "Random.h"

#define BEE P32

extern __code uint32_t LedData[];

void main(void)
{
  // 设置P5.4 P5.3 P3.3为推挽输出
  // P3.1 P3.0为准双向口
  P5M0 = 0x00; // 0b00110000
  P5M1 = 0x00; // 0b00000000

  P3M0 = 0x04; // 0b00001000
  P3M1 = 0x00;

  RandomSeedInit(RandomGet());

  Led_Init();

  BeepInit();

  while (1)
  {
    Led_write(0xffffffff, 32);
    delay_ms(1000);
    Led_write(0x00000000, 32);
    delay_ms(1000);

    for (uint8_t i = 0; i < 26; i++)
    {
      Led_Append(LedData[i], 32);
      BeepPlay(100 + i * 50, 200);
    }
    delay_ms(1000);
    
    Led_DisplayString("RUN", 2000);
    delay_ms(1000);
    Led_DisplayString("HELLO WORLD", 1000);

    for (uint8_t i = 0; i < 30; i++)
    {
      Led_write(RandomGetRange(0, 0x7FFFFFF) << 6, 32);
      BeepPlay(220, 100);
      delay_ms(400);
    }
    for (uint8_t i = 0; i < 30; i++)
    {
      Led_write(LedData[RandomGetRange(0, 25)], 32);
      BeepPlay(220, 100);
      delay_ms(400);
    }

    // Kids
    // PlayMusic(Music1, 0, 180, 3);
    // delay_ms(2000);
    // Running Up That Hill
    // PlayMusic(Music2, 0, 15, 3);
    // BeepSetFreq(180);
    // delay_ms(10);

    // PlayMusic(Music, 0, 80, 2);
    // delay_ms(2000);
  }
}
