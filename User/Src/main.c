#include "stc8_sdcc.h"
#include "stdint.h"
#include "Beep.h"
#include "Delay.h"
#include "Music.h"
#include "Music_source.h"
#include "Led.h"
#include "Random.h"
#include "Key.h"

#define BEE P32

typedef enum
{
  LED_MODE_OFF = 0,
  LED_MODE_ON = 1,
  LED_MODE_LOOP = 2,
  LED_MODE_RANDOM = 3,
  LED_RUN = 4,
} LedMode;

typedef enum
{
  MUSIC_MODE_OFF = 0,
  MUSIC_MODE_1,
  MUSIC_MODE_2,
  MUSIC_MODE_3,
} Music_Mode;

void Key_A1_ShortClick_callback();
void Key_A1_LongClick_callback();
void Key_A1_LongRepeat_callback();

LedMode led_mode = LED_MODE_OFF;
Music_Mode music_mode = MUSIC_MODE_OFF;

extern __code uint32_t LedData[];
extern uint32_t Led_DisplayData;
extern volatile Key key_A1;
extern volatile uint8_t tick_status;

void main(void)
{
  // 设置P5.4 P5.3 P3.3为推挽输出
  // P3.1 P3.0为准双向口
  P5M0 = 0x00; // 0b00110000
  P5M1 = 0x00; // 0b00000000

  P5M1 |= 0x20;

  P3M0 = 0x04; // 0b00001000
  P3M1 = 0x00;

  P5PU = 0x00;

  RandomSeedInit(RandomGet());

  Led_Init();

  BeepInit();

  BeepPlay(1000, 500);

  Led_write(0xffffffff, 32);
  delay_ms(1000);
  Led_write(0x00000000, 32);
  delay_ms(1000);
  uint8_t index = 0;

  while (1)
  {
    if (KEY1_PIN)
    {
      Led_write(LED_A, 32);
      // BeepPlay(1000, 100);
      while (KEY1_PIN)
        ;
      Led_write(Led_DisplayData & ~LED_A, 32);
      // BeepPlay(1000, 100);
      
      led_mode = (led_mode + 1) % 5;
      switch (led_mode)
      {
      case LED_MODE_OFF:
        Led_write(0x00000000, 32);
        break;

      case LED_MODE_ON:
        Led_write(0xFFFFFFFF, 32);
        break;

      default:
        break;
      }
    }

    switch (led_mode)
    {
    case LED_MODE_LOOP:
      Led_write(LedData[index], 32);
      index = (index + 1) % 26;
      delay_ms(180);
      break;
    
    case LED_MODE_RANDOM:
      Led_write(RandomGetRange(0, 0x7FFFFFF) << 6, 32);
      delay_ms(280);
      break;

    case LED_RUN:
      Led_write(0x00000000, 32);
      Led_DisplayString("RUN", 1000);
      Led_write(0x00000000, 32);
      break;
    default:
      break;
    }

    delay_ms(20);

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

// A1短按：LED模式切换
void Key_A1_ShortClick_callback()
{
  led_mode = (led_mode + 1) % 5;
  // Led_write(0x00000000, 32);
  // Led_DisplayChar('B' + led_mode);
  switch (led_mode)
  {
  case LED_MODE_OFF:
    Led_write(0UL, 32);
    break;

  case LED_MODE_ON:
    Led_write(0xFFFFFFFFUL, 32);
    break;

  default:
    break;
  }
}

void Key_A1_LongClick_callback()
{
}

// 保留
void Key_A1_LongRepeat_callback()
{
  ;
}