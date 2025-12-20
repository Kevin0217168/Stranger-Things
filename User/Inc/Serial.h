#ifndef __SERIAL_H__
#define __SERIAL_H__

#define BRT (65536 - 22118000 / 115200 / 4) // 假设这是你的波特率设置，根据实际需要调整

#include "stc8_sdcc.h"

void UartIsr() __interrupt 4;

void UartInit();

void UartSend(char data);

void UartSendString(char *string);

#endif
