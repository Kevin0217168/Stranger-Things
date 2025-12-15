#ifndef __SERIAL_H__
#define __SERIAL_H__

#define BRT (65536 - 22066000 / 115200 / 4) // 假设这是你的波特率设置，根据实际需要调整

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

void UartSendString(char *string){
  while(*string != '\0'){
    UartSend(*string);
    string++;
  }
}

#endif
