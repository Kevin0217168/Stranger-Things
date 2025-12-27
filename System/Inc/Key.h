#ifndef __KEY_H__
#define __KEY_H__

#include "stc8_sdcc.h"
#include "stdint.h"

#define SYS_CLOCK_HZ 22168000
// #define SYS_CLOCK_HZ 7372800
#define SYSTICK_FREQUENCY_HZ 1000

#define SYS_1MS_TASK 0b00000001
#define SYS_10MS_TASK 0b00000010
#define SYS_100MS_TASK 0b00000100
#define SYS_1000MS_TASK 0b00001000
#define SYS_20MS_TASK 0b00010000

#define KEY1_PIN P55

// 按键状态
typedef enum
{
    KEY_RELEASED = 0, // 按键松开(0)
    KEY_PRESSED,      // 按键按下(1)
} Key_status;

// 记录按键多次状态
typedef struct
{
    Key_status current_state;
    Key_status last_state;
} Key_state;

// 按键触发任务
typedef enum
{
    none,
    short_click_task,
    long_click_task,
    long_repeat_task,
} Key_task;

// 按键扫描状态机
typedef enum
{
    unstable_down,
    down,
    unstable_up,
    up,
} KeyProcess_status;

// 按键类
typedef struct
{
    Key_state state;                  // 按键多次状态
    KeyProcess_status process_status; // 状态机
    uint32_t press_currentTime;       // 记录从稳定按下到现在的时间(ms)
    uint8_t repeat_count;             // 连续计数，当长按超过一定值时，该值随扫描频率自增
    Key_task task;                    // 按键触发任务

    // 按键回调函数绑定，由Key_Process自动调用
    void (*shortClick_callback)(void);
    void (*longClick_callback)(void);
    void (*longRepeat_callback)(void);
} Key;

extern void Key_A1_ShortClick_callback();
extern void Key_A1_LongClick_callback();
extern void Key_A1_LongRepeat_callback();

void SysTickInit(void);
void SysTick_Handler(void) __interrupt 3;
uint32_t GetSysTick(void);
void Key_Init();
void Key_ScanProcess(volatile Key *key);
void Key_TriggerProcess(volatile Key *key);

#endif // __KEY_H__
