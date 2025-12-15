/**
 * 音乐播放器 - 适配STC8G1K08A + SDCC
 * 说明：使用PWM控制蜂鸣器发声，支持多种音乐格式
 */

#include "stdint.h"
#include "stdbool.h"
#include "stc8_sdcc.h"
#include "Beep.h"
#include "Delay.h"
#include "Music_source.h"  // 包含音乐数据

#define ROM __code
#define XDATA __xdata


// ==================== 硬件引脚定义 ====================
#define KEY_PIN         P55    // 按键引脚

// ==================== 全局变量 ====================
volatile uint8_t g_currentState = 0;  // 当前状态
volatile bool g_isPlaying = false;    // 播放标志
volatile bool g_keyPressed = false;   // 按键标志

// ==================== 频率表 ====================
// 中央C调频率表 (C4-B4)
ROM const uint16_t NOTE_FREQUENCIES[] = {
    // C   C#  D   D#  E   F   F#  G   G#  A   A#  B
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,  // 中音区
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,  // 高音区
    131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247   // 低音区
};

// 音符在频率表中的位置映射 (1-7 -> 0-11)
ROM const uint8_t NOTE_POSITIONS[] = {0, 2, 4, 5, 7, 9, 11};


// ==================== 工具函数 ====================

/**
 * @brief 解码音符获取频率
 * @param encodedNote 编码后的音符 (格式: SHM)
 *        S: 百位 - 升半音 (0-不升, 1-升)
 *        H: 十位 - 音区 (1-低音, 2-中音, 3-高音)
 *        M: 个位 - 音符 (1-7)
 * @param keySignature 调号 (0-11, 表示升多少个半音)
 * @return 频率值，0表示休止符
 */
uint16_t DecodeNoteFrequency(uint8_t encodedNote, uint8_t keySignature) {
    // 解码各个部分
    uint8_t note = encodedNote % 10;           // 音符 (1-7)
    uint8_t octave = (encodedNote / 10) % 10;  // 音区 (1-3)
    uint8_t sharp = encodedNote / 100;         // 是否升半音 (0/1)
    
    // 休止符处理
    if (note == 0) {
        return 0;
    }
    
    // 计算在频率表中的位置
    uint8_t position = NOTE_POSITIONS[note - 1] + sharp + keySignature;
    
    // 根据音区调整位置
    uint8_t baseIndex;
    switch (octave) {
        case 1:  // 低音区
            baseIndex = 24;  // 低音区起始位置
            break;
        case 2:  // 中音区
            baseIndex = 0;   // 中音区起始位置
            break;
        case 3:  // 高音区
            baseIndex = 12;  // 高音区起始位置
            break;
        default:
            baseIndex = 0;   // 默认为中音区
    }
    
    // 确保位置在有效范围内
    if (position >= 12) {
        position -= 12;
        baseIndex += 12;  // 升一个八度
    }
    
    uint8_t finalIndex = baseIndex + position;
    
    // 确保索引不越界
    if (finalIndex >= sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0])) {
        return 0;  // 无效音符
    }
    
    return NOTE_FREQUENCIES[finalIndex];
}

/**
 * @brief 解码时值获取播放时间(ms)
 * @param encodedDuration 编码后的时值
 * @param baseTempo 基准速度 (BPM)
 * @param articulation 演奏法 (0-普通, 1-连音, 2-顿音)
 * @return 播放时间(毫秒)
 */
uint16_t DecodeDuration(uint8_t encodedDuration, uint16_t baseTempo, uint8_t articulation) {
    // 解码
    uint8_t length = encodedDuration % 10;           // 时值 (0-6)
    uint8_t effect = (encodedDuration / 10) % 10;    // 演奏效果 (0-2)
    uint8_t dotted = encodedDuration / 100;          // 符点 (0/1)
    
    // 时值表: 全音符、二分音符、四分音符...
    ROM const uint8_t DURATION_VALUES[] = {64, 32, 16, 8, 4, 2, 1};
    
    if (length >= sizeof(DURATION_VALUES) / sizeof(DURATION_VALUES[0])) {
        return 0;  // 无效时值
    }
    
    // 计算基本时长 (四分音符的毫秒数)
    uint32_t quarterNoteMs = 60000 / baseTempo;  // 60000ms / BPM
    
    // 计算该音符的时长
    uint32_t durationMs = quarterNoteMs * 4 / DURATION_VALUES[length];
    
    // 符点音符：增加一半的时长
    if (dotted) {
        durationMs = durationMs + (durationMs / 2);
    }
    
    // 应用演奏法
    uint16_t soundMs, silenceMs;
    switch (articulation) {
        case 0:  // 普通奏法 (80%发声，20%间隔)
            soundMs = durationMs * 8 / 10;
            silenceMs = durationMs - soundMs;
            break;
        case 1:  // 连音 (95%发声，5%间隔)
            soundMs = durationMs * 95 / 100;
            silenceMs = durationMs - soundMs;
            break;
        case 2:  // 顿音 (50%发声，50%间隔)
            soundMs = durationMs / 2;
            silenceMs = durationMs - soundMs;
            break;
        default:  // 默认普通奏法
            soundMs = durationMs * 8 / 10;
            silenceMs = durationMs - soundMs;
    }
    
    return soundMs;
}

// ==================== 音乐播放器核心 ====================

/**
 * @brief 播放单首音乐
 * @param music 音乐数据指针
 * @param keySignature 调号 (0-11)
 * @param tempo 演奏速度 (BPM)
 * @param articulation 演奏法 (0-普通, 1-连音, 2-顿音)
 */
void PlayMusic(ROM const MusicNote* music, uint8_t keySignature, uint16_t tempo, uint8_t articulation) {
    uint16_t i = 0;
    
    g_isPlaying = true;
    
    while (g_isPlaying) {
        // 读取音符
        uint8_t note = music[i].note;
        uint8_t duration = music[i].duration;
        
        // 结束标志
        if (note == 0 && duration == 0) {
            break;
        }
        
        // 解码频率
        uint16_t frequency = DecodeNoteFrequency(note, keySignature);
        
        // 解码时长
        uint16_t playTime = DecodeDuration(duration, tempo, articulation);
        
        // 播放音符
        if (frequency > 0) {
            BeepSetFreq(frequency);
        } else {
            BeepSetFreq(0);  // 休止符，停止发声
        }
        
        // 播放持续
        for (uint16_t t = 0; t < playTime; t += 10) {
            delay_ms(10);
            
            // // 检查按键中断
            // if (!KEY_PIN) {  // 按键按下
            //     delay_ms(20);  // 消抖
            //     if (!KEY_PIN) {
            //         g_isPlaying = false;
            //         g_keyPressed = true;
            //         break;
            //     }
            // }
        }
        
        // 音符间隔 (顿音、连音等已包含在DecodeDuration中)
        
        i++;
    }
    
    // 停止发声
    BeepSetFreq(0);
    g_isPlaying = false;
}

/**
 * @brief 音乐播放管理器
 */
void MusicPlayerManager(void) {
    while (1) {
        // 等待按键
        while (KEY_PIN);  // 等待按键释放
        
        if (g_keyPressed) {
            g_keyPressed = false;
            g_currentState = (g_currentState + 1) % 3;  // 循环3首音乐
        }
        
        // 根据状态播放对应的音乐
        switch (g_currentState) {
            case 0:
                PlayMusic(MUSIC_2, 0, 120, 0);  // C调，120BPM，普通奏法
                break;
            case 1:
                PlayMusic(MUSIC_3, 0, 100, 0);  // C调，100BPM，普通奏法
                break;
            case 2:
                PlayMusic(MUSIC_4, 0, 140, 0);  // C调，140BPM，普通奏法
                break;
        }
    }
}

/**
 * @brief 初始化函数
 */
void MusicPlayer_Init(void) {
    // 初始化变量
    g_currentState = 0;
    g_isPlaying = false;
    g_keyPressed = false;
}

// 选择要测试的音符
void TestSingleNote(uint8_t noteCode, uint16_t duration_ms) {
    // 解码音符获取频率
    uint16_t freq = DecodeNoteFrequency(noteCode, 0);  // 调号为0
    
    // 设置频率
    BeepSetFreq(freq);
    
    // 播放指定时间
    for(uint16_t i = 0; i < duration_ms; i++) {
        delay_ms(1);
    }
    
    // 停止发声
    SetFreq(0);
}


// ==================== 示例主函数 ====================
/*
void main(void) {
    // 系统初始化
    System_Init();
    
    // 音乐播放器初始化
    MusicPlayer_Init();
    
    // 进入音乐播放管理器
    MusicPlayerManager();
    
    while (1);
}
*/
