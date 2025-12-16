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

// uint8_t string[] = "Hello World!\r\n\0";

// // 测试几个关键值，看看解码结果是什么
// void DebugDecoding(void) {
//   uint8_t test_values[] = {
//     0x19, 0x02, // D4 quarter note
//     0x1A, 0x03, // D#4 eighth note
//     0x1B, 0x04, // E4 half note
//     0x10, 0x05, // C4 dotted quarter note
//     0x00, 0x00  // End
//   };
//   for (uint8_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
//     uint8_t val = test_values[i];
//     uint8_t delay = test_values[++i];
    
//     uint16_t freq = DecodeNoteFrequency(val, 0, 0);
    
//     DEBUG_LOG("Note Code: %x => Frequency: %d Hz", val, freq);

//     MusicNote musicNote;
//     DecodeDuration(&musicNote, delay, 100); // 假设120 BPM，普通奏法
//     DEBUG_LOG("soundMs=%d, silenceMs=%d\r\n",musicNote.soundMs, musicNote.silenceMs);
    
//     // 实际播放，用示波器测量
//     BeepSetFreq(freq);
//     delay_ms(3000);
//     BeepSetFreq(0);
//     delay_ms(1000);
//   }
//   delay_ms(10000);
// }

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

  UartInit();
  UartSendString("init done\n\r");
  
  BeepInit();
  // MusicPlayer_Init();

  UartSendString("BeepInit\n\r");
  write(0x000000, 24);

  while(1){
    // UartSendString(string);
    // write(0xffffff, 24);
    // delay_ms(1000);
    // write(0x000000, 24);
    // delay_ms(1000);
    
    // CR = 1;
    // for (uint8_t i = 0; i < 24; i++){
    //   write(1UL << 1UL * i, 24);
    //   BeepSetFreq(100 + i * 100);
    //   delay_ms(500);
    // }
    // CR = 0;

    // MusicPlayerManager();

    // DebugDecoding();

    // Kids
    PlayMusic(Music1, 0, 180, 3);
    delay_ms(2000);
    // Running Up That Hill
    PlayMusic(Music2, 0, 15, 3);

    // PlayMusic(Music, 0, 80, 2);
    delay_ms(2000);
  }
}

