// ============================================================
//  tts.h  —  Text-to-Speech via Google Translate TTS
// ============================================================
#pragma once
#include <Arduino.h>

// Start streaming TTS for the given text.
// Non-blocking — audio plays via ttsAudio.loop() in audioTick().
// Check ttsDone() to know when playback has finished.
void startTTS(String text);

// Returns true when the last startTTS() call has finished playing.
// Also returns true before any startTTS() call (initial state).
bool ttsDone();

// NOTE: The ESP32-audioI2S callbacks (audio_eof_mp3, audio_info,
// audio_error) are defined in tts.cpp. They are global weak symbols
// called automatically by the library — do NOT declare them here
// or the linker will see duplicate definitions if this header is
// included in multiple translation units.
