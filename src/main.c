#include "stc8_sdcc.h"
#include "Serial.h"
#include "stdint.h"

#define DS P31
#define ST_CP P30
#define SH_CP P33
#define BEE P32


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

void pwm_init(void){
  // 设置PCA功能脚，将CCP0 绑定到P32（BEE）
  P_SW1 = 0x00;
  // 设置PCA脉冲源：系统时钟，禁用溢出中断，空闲下继续计数
  CMOD = 0x08;
  // 初始化计数器
  CL = 0x00;
  CH = 0x00;
  // 启用PCA0 6/7/8/10 位PWM 脉冲输出 无中断
  CCAPM0 = 0x42;
  // 设置输出6位PWM
  // 当设置EPCnH和EPCnL = 1时，计数器会始终小于比较值，始终输出低电平
  PCA_PWM0 = 0x80;
  // 设置比较值
  // 6位下，计数器CL取低六位，最大64
  // 比较值b100000 = 0x20 = 32 刚好为一半，占空比50%
  // 同时计数器溢出一次，翻转一次，完成一个周期，即频率=PCA脉冲/计数器溢出值64
  CCAP0L = 0x10;  
  // 设置比较器重装载值（影子寄存器）
  CCAP0H = 0x10;
  // 输出开启
  // CCON = 0x40;
  CR = 1;
}

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
  pwm_init();

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

