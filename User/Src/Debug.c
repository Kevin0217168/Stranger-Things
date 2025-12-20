#include "Debug.h"
#include "stdint.h" 
#include <stdarg.h>
// #include "Serial.h"

extern void UartSend(char c);

void mini_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buf[10];
    uint32_t val;
    const char *str;
    char c;
    char *p;
    
    while (*fmt) {
        if (*fmt != '%') {
            UartSend(*fmt++);
            continue;
        }
        
        fmt++;
        switch (*fmt) {
            case 'd':
                val = (uint32_t)va_arg(args, int);
                if ((int32_t)val < 0) {
                    UartSend('-');
                    val = (uint32_t)(-(int32_t)val);
                }
                p = buf + 9;
                *p = '\0';
                do {
                    *--p = '0' + (val % 10);
                    val /= 10;
                } while (val > 0);
                while (*p) UartSend(*p++);
                break;
                
            case 'x':
                val = va_arg(args, uint16_t);
                UartSend('0');
                UartSend('x');
                p = buf + 8;
                *p = '\0';
                do {
                    uint8_t digit = val & 0x0F;
                    *--p = digit < 10 ? '0' + digit : 'a' + digit - 10;
                    val >>= 4;
                } while (val > 0);
                while (*p) UartSend(*p++);
                break;
                
            case 's':
                str = va_arg(args, const char*);
                if (str) while (*str) UartSend(*str++);
                break;
                
            case 'c':
                c = (char)va_arg(args, int);
                UartSend(c);
                break;
        }
        fmt++;
    }
    
    va_end(args);
}