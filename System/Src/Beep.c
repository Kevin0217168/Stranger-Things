#include "stc8_sdcc.h"
#include <stdint.h>
#include "Beep.h"
#include "Delay.h"
#include "Key.h"

void BeepSetFreq(uint16_t freq){
  if (freq == 0){
    // 停止发声
    CR = 0;
    return;
  }else if(!CR){
    CR = 1;
  }
  // 使用整数算式计算重装值，避免浮点运算
  // 目标：ticks = SYS_CLOCK_HZ / (64 * freq) - 2
  // 使用四舍五入：ticks = (SYS_CLOCK_HZ + denom/2) / denom
  uint32_t denom = (uint32_t)64U * (uint32_t)freq;
  uint32_t ticks = 0;
  if (denom != 0U) {
    ticks = (SYS_CLOCK_HZ + denom / 2U) / denom;
    if (ticks >= 2U) {
      ticks -= 2U;
    } else {
      ticks = 0U;
    }
  }
  uint16_t count = (uint16_t)(65535U - (uint16_t)ticks);
  TH0 = count >> 8;
  TL0 = count & (uint16_t)0x00ff;
}

void timer0_init(void){
  // 设置TCON进行定时器设置
  // 由于TCON寄存器可位寻址，可直接操作位
  TR0 = 1; // 开启计时器0
  TMOD &= 0xf0; // 设置计时器0为16位自动重装载计时器
  
  // 对AUXR第7位置1 设置时钟分频值为1
  AUXR = AUXR | 0x80;
}

void pwm_init(void){
  // 设置PCA功能脚，将CCP0 绑定到P32（BEE）
  P_SW1 = 0x00;
  // 设置PCA脉冲源：使用计时器0溢出信号，禁用溢出中断，空闲下继续计数
  CMOD = 0x04;
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
  CCAP0L = 0x20;  
  // 设置比较器重装载值（影子寄存器）
  CCAP0H = 0x20;
  // 输出开启
  // CCON = 0x40;
  CR = 1;
}

void BeepInit(){
  timer0_init();
  pwm_init();
  BeepSetFreq(0);
}

BeepTask beepTask = {0, BEEP_OFF};

void BeepPlay(uint16_t freq, uint16_t duration_tick){
  beepTask.state = BEEP_ON;
  beepTask.end_tick = GetSysTick() + duration_tick;
  BeepSetFreq(freq);
}

void BeepProcess(BeepTask* beepTask){
  if (beepTask->state == BEEP_ON && GetSysTick() >= beepTask->end_tick){
    BeepSetFreq(0);
    beepTask->state = BEEP_OFF;
  }
}