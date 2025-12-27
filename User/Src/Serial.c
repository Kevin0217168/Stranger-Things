#include "Serial.h"
#include "stc8_sdcc.h"

__bit  busy;

void UartIsr() __interrupt 4{
  if (TI){
    TI = 0;
    busy = 0;
  }
  if (RI){
    RI = 0;
  }
}

#ifdef UART_TIMER_1

void UartInit(){
  SCON = 0x50;
  TMOD = 0x00;
  TL1 = BRT;
  TH1 = BRT >> 8;
  TR1 = 1;
  AUXR = 0x40;
  busy = 0;
  ES = 1;
  EA = 1;
}

void UartSend(char data){
  while(busy);
  busy = 1;
  SBUF = data;
}

#elif defined UART_TIME_FIXED

void UartInit(void)
{
    // 串口1模式2配置
    SCON = 0x90;    // 1001 0000
                    // SM0=1, SM1=0 → 模式2 (9位UART)
                    // REN=1 → 允许接收
    
    // 波特率设置
    PCON = 0x00;    // SMOD=0 → 波特率=Fosc/64=115200
    
    // 中断使能（可选）
    ES = 1;         // 使能串口1中断
    EA = 1;         // 使能总中断
}

// 发送数据（8位数据，第9位固定为0）
void UartSend(char dat)
{
    TB8 = 1;        // 第9位设为0（固定）
    while(busy);
    busy = 1;
    SBUF = dat;
}

#endif

void UartSendString(char *string){
  while(*string != '\0'){
    UartSend(*string);
    string++;
  }
}
