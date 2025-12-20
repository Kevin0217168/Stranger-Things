#include "stc8_sdcc.h"
#include "stdint.h"

#define DEBUG_ENABLE 1

#include "Serial.h"
#include "Beep.h"
#include "Delay.h"
#include "Music.h"
#include "Debug.h"
#include "Music_source.h" // 包含音乐数据

#define DS P54
#define ST_CP P33
#define SH_CP P30
#define BEE P32

void write(uint32_t data, uint8_t len){
  // 先定义掩码，每次只移一位，速度更快
  uint32_t hide_code = 1UL;
  for (int bit_index = 0; bit_index < len; bit_index++){
    SH_CP = 0;
    // __asm__("nop");
    DS = data & hide_code; // 掩码必须说明为uint32_t否则会出错
    // __asm__("nop");
    SH_CP = 1;
    // __asm__("nop");
    hide_code = hide_code << 1;
  }
  DS = 0;
  SH_CP = 0;
  __asm__("nop");
  ST_CP = 1;
  __asm__("nop");
  ST_CP = 0;
}

void main(void)
{
  // 设置P5.4 P5.3 P3.3为推挽输出
  // P3.1 P3.0为准双向口
  P5M0 = 0x00; // 0b00110000
  P5M1 = 0x00; // 0b00000000

  P3M0 = 0x04; // 0b00001000
  P3M1 = 0x00;
  
  DS = 0;
  SH_CP = 0;
  ST_CP = 0;

  delay_ms(500);

  // UartInit();
  // UartSendString("init done\n\r");
  
  BeepInit();
  // MusicPlayer_Init();
  BeepSetFreq(500);
  // UartSendString("BeepInit\n\r");
  write(0x000000, 24);

  while(1){
    // UartSendString(string);
     write(0xffffffff, 32);
     delay_ms(1000);
     write(0x00000000, 32);
     delay_ms(1000);
    
     CR = 1;
     for (uint8_t i = 0; i < 26; i++){
       write(1UL << 1UL * i, 26);
       BeepSetFreq(100 + i * 100);
       delay_ms(500);
     }
     CR = 0;

    // MusicPlayerManager();

    // DebugDecoding();

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

