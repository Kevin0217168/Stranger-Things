#include "Key.h"
#include "Led.h"

extern uint32_t Led_DisplayData;

volatile uint32_t sysTick = 0;
volatile uint8_t tick_status = 0;

void SysTickInit(void)
{
  TR1 = 1;
  TMOD &= ~T1_CT;
  TMOD &= ~(T1_M1 | T1_M0);
  AUXR |= T1x12;

  uint16_t reloadValue = 65536 - (SYS_CLOCK_HZ / SYSTICK_FREQUENCY_HZ);
  TH1 = (uint8_t)(reloadValue >> 8);
  TL1 = (uint8_t)(reloadValue & 0xFF);

  ET1 = 1;
  EA = 1;
}

void SysTick_Handler(void) __interrupt 3
{
  sysTick++;

  tick_status |= SYS_1MS_TASK;
  if (GetSysTick() % 10 == 0) {
    tick_status |= SYS_10MS_TASK;
  }if (GetSysTick() % 100 == 0) {
    tick_status |= SYS_100MS_TASK;
  }if (GetSysTick() % 1000 == 0) {
    tick_status |= SYS_1000MS_TASK;
  }if (GetSysTick() % 20 == 0) {
    tick_status |= SYS_20MS_TASK;
  }
}

uint32_t GetSysTick(void)
{
  return sysTick;
}

// 按键类
volatile Key key_A1;

// 按键类默认值初始化(私有)
void keyStruct_init(volatile Key *key)
{
  key->state.current_state = KEY_RELEASED;
  key->state.last_state = KEY_RELEASED;
  key->press_currentTime = 0;
  key->repeat_count = 0;
  key->process_status = up;
  key->task = none;
}

void Key_Init()
{
  key_A1.shortClick_callback = &Key_A1_ShortClick_callback;
  key_A1.longClick_callback = &Key_A1_LongClick_callback;
  key_A1.longRepeat_callback = &Key_A1_LongRepeat_callback;
  keyStruct_init(&key_A1);
}

/**
 * 获取按键值(私有)
 * @param key 要获取的按键对象
 * @return KEY_PRESSED:按下(1) KEY_RELEASED:松开(0)
 */
Key_status getKeyState() {
  if (KEY1_PIN == 1) {
    return KEY_PRESSED;
  }
  return KEY_RELEASED;
}

/**
 * 扫描按键函数，自动设置按键任务标志
 * @param key 要获取的按键对象
 */
void Key_ScanProcess(volatile Key* key) {
  // 获取键值
  key->state.current_state = getKeyState();

  switch (key->process_status) {
    case up:
      if (key->state.last_state == KEY_RELEASED && key->state.current_state == KEY_PRESSED) {
        key->process_status = unstable_down;
      }
      break;

    case unstable_down:
      // 记录按下时间
      key->press_currentTime = GetSysTick();
      // 下一个状态
      if (key->state.last_state == KEY_PRESSED && key->state.current_state == KEY_PRESSED) {
        key->process_status = down;
      }else {
        key->process_status = up;
      }
      break;

    case down:
      // 判断持续长按500ms后触发快速重复任务
      if (GetSysTick() - key->press_currentTime > 1000) {
        // 若轮询间隔为20ms，则重复任务为100ms一次
        if (key->repeat_count % 5 == 0) {
          key->task = long_repeat_task;
        }
        key->repeat_count++;
      }

      // 下一个状态
      if (key->state.last_state == KEY_PRESSED && key->state.current_state == KEY_RELEASED) {
        key->process_status = unstable_up;
      }
      break;

    case unstable_up:
      // 下一个状态
      if (key->state.last_state == KEY_RELEASED && key->state.current_state == KEY_RELEASED) {
        // 按键稳定松开，判断长短，发布点击任务
        if  (GetSysTick() - key->press_currentTime <= 200) {
          key->task = short_click_task;
        }else {
          key->task = long_click_task;
          key->repeat_count = 0;
        }
        key->process_status = up;
      }else {
        key->process_status = down;
      }
      break;
  }

  key->state.last_state = key->state.current_state;
}


/**
 * 处理按键任务函数，调用对应任务的回调函数，自动清除按键标志位
 * @param key 要获取的按键对象
 */
void Key_TriggerProcess(volatile Key* key) {
  switch (key->task) {
    case short_click_task:
      key->shortClick_callback();
      key->task = none;
      break;

    case long_click_task:
      key->longClick_callback();
      key->task = none;
      break;

    case long_repeat_task:
      key->longRepeat_callback();
      key->task = none;
      break;

    default:
      ;
  }
}