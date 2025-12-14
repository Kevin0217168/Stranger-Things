#include "stc8_sdcc.h"
#include "Serial.h"
#include "stdint.h"

#define DS P31
#define ST_CP P30
#define SH_CP P33

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 1000; j++){
          __asm__("nop");
        }
}

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

uint8_t string[] = "Hello World!\n\r";

void main(void)
{
  // 设置P5.4 P5.3 P3.3为推挽输出
  // P3.1 P3.0为准双向口
  P5M0 = 0x30; // 0b00110000
  P5M1 = 0x00; // 0b00000000

  P3M0 = 0x00; // 0b00001000
  P3M1 = 0x00;
  
  DS = 0;
  SH_CP = 0;
  ST_CP = 0;

  // UartInit();

  while(1){
    //UartSendString(string);
    write(0xffffff, 24);
    delay_ms(1000);
    write(0x000000, 24);
    delay_ms(1000);

    for (uint8_t i = 0; i < 24; i++){
      write(1UL << 1UL * i, 24);
      delay_ms(100);
    }
  }
}

