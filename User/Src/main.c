#include "stc8_sdcc.h"
#include "stdint.h"
#include "Beep.h"
#include "Delay.h"
#include "Music.h"
#include "Music_source.h"
#include "Led.h"
#include "Random.h"
#include "Key.h"
#include "Serial.h"

#define BEE P32

typedef enum{
  LED_MODE_OFF = 0,
  LED_MODE_ON = 1,
  LED_MODE_LOOP = 2,
  LED_MODE_RANDOM = 3,
  LED_RUN = 4,
} LedMode;

typedef enum{
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
extern LedTask ledTask;
extern BeepTask beepTask;
extern MusicPlayTask musicPlayTask;
int8_t index = 0;

void LED_loop(){
  if (index <= 25){
    Led_Append(LedData[index], 32);
  }else if (index <= 35){
    ;
  }else{
    Led_write(0UL, 32);
    index = -1;
  }
  index++;
}

void LED_random(){
  if (index > 1){
    Led_write(RandomGetRange(0, 0x7FFFFFF) << 6, 32);
    index = 0;
  }
  index++;
}

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

  UartInit();
  delay_ms(2000);
  UartSendString("System Start...\r\n");

  RandomSeedInit(RandomGet());

  Led_Init();

  BeepInit();

  SysTickInit();

  Key_Init();

  Led_write(0xffffffff, 32);
  delay_ms(1000);
  Led_write(0x00000000, 32);
  delay_ms(1000);

  Led_DisplayChar('A');
  while (1)
  {
    if (tick_status & SYS_1MS_TASK)
    {
      BeepProcess(&beepTask);
      Led_DisplayStringProcess(&ledTask);
      MusicPlayProcess(&musicPlayTask);

      tick_status &= ~SYS_1MS_TASK;
    }
    if (tick_status & SYS_10MS_TASK)
    {
      
      tick_status &= ~SYS_10MS_TASK;
    }
    if (tick_status & SYS_100MS_TASK)
    {
      // 执行100ms任务
      Key_TriggerProcess(&key_A1);
      

      switch(led_mode){
        case LED_MODE_LOOP:
          LED_loop();
          break;
        case LED_MODE_RANDOM:
          LED_random();
          break;
        default:
          break;
      }

      tick_status &= ~SYS_100MS_TASK;
    }
    if (tick_status & SYS_1000MS_TASK)
    {
      // 执行1000ms任务

      tick_status &= ~SYS_1000MS_TASK;
    }
    if (tick_status & SYS_20MS_TASK)
    {
      // 扫描按键
      Key_ScanProcess(&key_A1);

      tick_status &= ~SYS_20MS_TASK;
    }

    // for (uint8_t i = 0; i < 30; i++)
    // {
    //   Led_write(LedData[RandomGetRange(0, 25)], 32);
    //   BeepPlay(220, 100);
    //   delay_ms(400);
    // }
  }
}

// A1短按：LED模式切换
void Key_A1_ShortClick_callback(){
  led_mode = (led_mode + 1) % 5;
  switch (led_mode)
  {
    case LED_MODE_OFF:
      Led_write(0UL, 32);
      break;

    case LED_MODE_ON:
      Led_write(0xFFFFFFFFUL, 32);
      break;

    case LED_MODE_LOOP:
      Led_write(0x00000000, 32);
      index = 0;
      break;

    case LED_MODE_RANDOM:
      Led_write(0x00000000, 32);
      index = 0;
      break;

    case LED_RUN:
      Led_write(0x00000000, 32);
      Led_DisplayString("RUN", 800);
      break;
      
    default:
      break;
  }
}


void Key_A1_LongClick_callback(){
  music_mode = (music_mode + 1) % 4;
  switch (music_mode){
    case MUSIC_MODE_1:
      // Kids
      PlayMusic(Music1, 0, 180, 3);
      break;
    case MUSIC_MODE_2:
      // Running Up That Hill
      PlayMusic(Music2, 0, 15, 3);
      break;
    case MUSIC_MODE_3:
      PlayMusic(Music, 0, 80, 2);
      break;
    default:
      musicPlayTask.mode = MUSIC_PLAY_OFF;
      break;
  }
}

// 保留
void Key_A1_LongRepeat_callback()
{
    ;
}