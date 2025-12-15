#ifndef __MUSIC_H__
#define __MUSIC_H__

typedef struct {
    uint8_t note;      // 音符编码
    uint8_t duration;  // 时值编码
} MusicNote;

void MusicPlayerManager(void);
void MusicPlayer_Init(void);
void TestSingleNote(uint8_t noteCode, uint16_t duration_ms);
uint16_t DecodeNoteFrequency(uint8_t encodedNote, uint8_t keySignature);

#endif
