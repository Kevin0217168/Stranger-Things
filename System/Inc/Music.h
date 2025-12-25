#ifndef __MUSIC_H__
#define __MUSIC_H__

// 新的频率表
typedef struct
{
    uint16_t NewFreTab[12];
} MusicTable;

typedef struct
{
    uint16_t freq;      // 音符编码
    uint16_t soundMs;   // 时值编码
    uint16_t silenceMs; // 间隔时值编码
} MusicNote;

typedef enum{
    MUSIC_PLAY_ON,
    MUSIC_PLAY_OFF,
} MusicPlayMode;

typedef struct{
    MusicPlayMode mode;
    __code uint8_t *music;
    uint8_t keySignature;
    uint16_t tempo;
    uint8_t octave;

    uint16_t i;
    uint32_t nextTick;
} MusicPlayTask;

void SetMusicTable(uint8_t Signature, uint8_t Octachord);
uint16_t DecodeNoteFrequency(uint8_t encodedNote, uint8_t keySignature, uint8_t octave);
void DecodeDuration(MusicNote *musicNote, uint8_t encodedDuration, uint16_t baseTempo);

void PlayMusic(__code uint8_t *music, uint8_t keySignature, uint16_t tempo, uint8_t octave);

void MusicPlayProcess(MusicPlayTask *task);

#endif
