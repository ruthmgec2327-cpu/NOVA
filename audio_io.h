#pragma once
#include <Arduino.h>
#include <Audio.h>
#include <driver/i2s_std.h>

extern Audio ttsAudio;

void     audioInit();
void     audioTick();
bool     isAudioRunning();
void     playActivationBeep();
void     startRecording();
bool     recordingDone();
void     stopRecording();
int16_t* getRecordingBuffer();
int      getRecordingSamples();
void     pauseMic();
void     resumeMic();
