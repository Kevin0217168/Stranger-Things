/**
 * 音乐播放器 - 适配STC8G1K08A + SDCC
 * 说明：使用PWM控制蜂鸣器发声，支持多种音乐格式
 */

#include "stdint.h"
#include "stdbool.h"
#include "stc8_sdcc.h"
#include "Beep.h"
#include "Delay.h"
#include "Music.h"
// #include "Music_source.h" // 包含音乐数据
#include "Debug.h"

#define ROM __code
#define XDATA __xdata

// 0x19 25
// ==================== 硬件引脚定义 ====================
#define KEY_PIN         P55    // 按键引脚

// ==================== 频率表 ====================
// 中央C调频率表 (C4-B4)
ROM const uint16_t FreTab[] = {
    // C   C#  D   D#  E   F   F#  G   G#  A   A#  B
    262,
    277,
    294,
    311,
    330,
    349,
    370,
    392,
    415,
    440,
    466,
    494, // 中音区
};

// 音符在频率表中的位置映射 (1-7 -> 0-11)
ROM const uint8_t SignTab[] = {0, 2, 4, 5, 7, 9, 11};

// 时值表: 全音符、二分音符、四分音符...
ROM const uint8_t DURATION_VALUES[] = {64, 32, 16, 8, 4, 2, 1};

// ==================== 工具函数 ====================

MusicTable musicTable;

/**
 * @brief 设置音乐频率表
 * @param Signature 调号 (0-11)
 * @param Octachord 升降八度 (1-低音, 2-中音, 3-高音)
 */
void SetMusicTable(uint8_t Signature, uint8_t Octachord)
{

    // 根据调号及升降八度来生成新的频率表
    for (uint8_t i = 0; i < 12; i++)
    {
        uint8_t j = i + Signature;
        if (j > 11)
        {
            j = j - 12;
            musicTable.NewFreTab[i] = FreTab[j] * 2;
        }
        else
            musicTable.NewFreTab[i] = FreTab[j];

        if (Octachord == 1)
            musicTable.NewFreTab[i] >>= 1;
        else if (Octachord == 3)
            musicTable.NewFreTab[i] <<= 1;

        mini_print("SetMusicTable: i=%d, j=%d, freq=%d\r\n", i, j, musicTable.NewFreTab[i]);
    }
}

/**
 * @brief 解码音符获取频率
 * @param encodedNote 编码后的音符 (格式: SHM)
 *       L: 音符 (1-7, 0=休止符)
 *       M: 八度 (1=低音, 2=中音, 3=高音)
 *       S: 升半音 (0=自然音, 1=升半音)
 * @return 频率 (Hz)
 */
uint16_t DecodeNoteFrequency(uint8_t encodedNote, uint8_t keySignature, uint8_t octave)
{
    uint8_t SL = encodedNote % 10;      // 计算出音符
    uint8_t SM = encodedNote / 10 % 10; // 计算出高低音
    uint8_t SH = encodedNote / 100;     // 计算出是否升半

    SetMusicTable(keySignature, octave);

    // 休止符处理
    if (SL == 0)
    {
        return 0;
    }
    
    uint16_t CurrentFre = musicTable.NewFreTab[SignTab[SL - 1] + SH]; // 查出对应音符的频率
    if (SM == 1)
        CurrentFre >>= 1; // 低音
    if (SM == 3)
        CurrentFre <<= 1; // 高音

    mini_print("note: %d, level: %d, symbol: %d, CurrentFre: %d\r\n", SL, SM, SH, CurrentFre);
        
    return CurrentFre;
}

/**
 * @brief 解码时值获取播放时间(ms)
 * @param musicNote 音符结构体指针
 * @param encodedDuration 编码后的时值
 * @param baseTempo 基准速度 (BPM)
 * @param articulation 演奏法 (0-普通, 1-连音, 2-顿音)
 * @return 播放时间(毫秒)
 */
void DecodeDuration(MusicNote *musicNote, uint8_t encodedDuration, uint16_t baseTempo)
{
    // 解码
    uint8_t length = encodedDuration % 10;        // 时值 (0-6)
    uint8_t effect = (encodedDuration / 10) % 10; // 演奏效果 (0-2)
    uint8_t dotted = encodedDuration / 100;       // 符点 (0/1)

    if (length >= sizeof(DURATION_VALUES) / sizeof(DURATION_VALUES[0]))
    {
        return ; // 无效时值
    }

    // 计算基本时长 (四分音符的毫秒数)
    uint32_t quarterNoteMs = 60000 / baseTempo; // 60000ms / BPM

    // 计算该音符的时长
    uint16_t durationMs = quarterNoteMs * 4 / DURATION_VALUES[length];

    // 符点音符：增加一半的时长
    if (dotted)
    {
        durationMs = durationMs + (durationMs / 2);
    }

    mini_print("DecodeDuration: length=%d, effect=%d, dotted=%d, durationMs=%d\r\n",
               length, effect, dotted, durationMs);

    // 应用演奏法
    switch (effect)
    {
    case 0: // 普通奏法 (80%发声，20%间隔)
        musicNote->soundMs = durationMs * 0.8;
        musicNote->silenceMs = durationMs - musicNote->soundMs;
        break;
    case 1: // 连音 (95%发声，5%间隔)
        musicNote->soundMs = durationMs * 0.95;
        musicNote->silenceMs = durationMs - musicNote->soundMs;
        break;
    case 2: // 顿音 (50%发声，50%间隔)
        musicNote->soundMs = durationMs * 0.5;
        musicNote->silenceMs = durationMs - musicNote->soundMs;
        break;
    default: // 默认普通奏法
        musicNote->soundMs = durationMs * 0.8;
        musicNote->silenceMs = durationMs - musicNote->soundMs;
    }
}

// ==================== 音乐播放器核心 ====================

/**
 * @brief 播放单首音乐
 * @param music 音乐数据指针
 * @param keySignature 调号 (0-11)
 * @param tempo 演奏速度 (BPM)
 * @param articulation 演奏法 (0-普通, 1-连音, 2-顿音)
 */
void PlayMusic(__code uint8_t *music, uint8_t keySignature, uint16_t tempo, uint8_t octave)
{
    uint16_t i = 0;
    while (music[i+1] != 0 || music[i] != 0)
    {
        // 读取音符
        uint8_t note = music[i];
        uint8_t duration = music[i+1];

        // 结束标志
        if (note == 0 && duration == 0)
        {
            break;
        }
        
        // 解码
        MusicNote musicNote;
        // 解码频率
        musicNote.freq = DecodeNoteFrequency(note, keySignature, octave);
        mini_print("PlayMusic: note=%x, freq=%d\r\n", note, musicNote.freq);

        // 解码时长
        DecodeDuration(&musicNote, duration, tempo);
        mini_print("PlayMusic: soundMs=%d, silenceMs=%d\r\n", musicNote.soundMs, musicNote.silenceMs);

        // 播放音符
        if (musicNote.freq > 0)
        {
            BeepSetFreq(musicNote.freq); // 设置频率
        }
        else
        {
            BeepSetFreq(0); // 休止符，停止发声
        }

        delay_ms(musicNote.soundMs); // 播放时长
        BeepSetFreq(0);              // 停止发声
        delay_ms(musicNote.silenceMs); // 间隔时长

        i += 2;
    }

    // 停止发声
    BeepSetFreq(0);
    // g_isPlaying = false;
}

// /**
//  * @brief 音乐播放管理器
//  */
// void MusicPlayerManager(void)
// {
//     while (1)
//     {
//         // 等待按键
//         while (KEY_PIN)
//             ; // 等待按键释放

//         if (g_keyPressed)
//         {
//             g_keyPressed = false;
//             g_currentState = (g_currentState + 1) % 3; // 循环3首音乐
//         }

//         // 根据状态播放对应的音乐
//         switch (g_currentState)
//         {
//         case 0:
//             PlayMusic(Music, 0, 120, 0); // C调，120BPM，普通奏法
//             break;
//         case 1:
//             PlayMusic(Music1, 0, 100, 0); // C调，100BPM，普通奏法
//             break;
//         case 2:
//             PlayMusic(Music2, 0, 140, 0); // C调，140BPM，普通奏法
//             break;
//         }
//     }
// }


// 选择要测试的音符
void TestSingleNote(uint8_t noteCode, uint16_t duration_ms)
{
    // 解码音符获取频率
    uint16_t freq = DecodeNoteFrequency(noteCode, 0, 0);

    // 设置频率
    BeepSetFreq(freq);

    // 播放指定时间
    for (uint16_t i = 0; i < duration_ms; i++)
    {
        delay_ms(1);
    }

    // 停止发声
    BeepSetFreq(0);
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
