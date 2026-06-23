// ============================================================
//  tts.cpp  —  Text-to-Speech via Google Translate TTS
//              Streamed as MP3 through ESP32-audioI2S
//
//  The Audio object (ttsAudio) lives in audio_io.cpp and is
//  declared extern in audio_io.h — we just use it here.
//  No second Audio instance, no second I2S driver.
//
//  ESP32-audioI2S callback functions MUST be defined at global
//  scope in a .cpp file — they are weak symbols that the
//  library calls automatically. Defining them here is correct.
// ============================================================
#include "tts.h"
#include "audio_io.h"    // gives us: extern Audio ttsAudio

bool _ttsDone = true;

// ── TTS API ──────────────────────────────────────────────────

void startTTS(String text) {
  if (text.length() == 0) { _ttsDone = true; return; }

  // Truncate to 200 chars — Google TTS URL limit
  if (text.length() > 200) text = text.substring(0, 200);

  Serial.println("[TTS] Calling connecttospeech...");
  ttsAudio.connecttospeech(text.c_str(), "en");
}

bool ttsDone() {
  return _ttsDone;
}
