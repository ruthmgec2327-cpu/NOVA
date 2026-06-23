// ============================================================
//  EvoAssistant — Main Entry Point & State Machine
//  ESP32-S3 AI Voice Assistant
//  Files: EvoAssistant.ino, config.h, oled.h, oled.cpp,
//         audio_io.h, audio_io.cpp, stt.h, stt.cpp,
//         tts.h, tts.cpp, ai_brain.h, ai_brain.cpp
// ============================================================

#include "config.h"
#include "oled.h"
#include "audio_io.h"
#include "stt.h"
#include "tts.h"
#include "ai_brain.h"

#include <WiFi.h>

// ── State Machine ────────────────────────────────────────────
enum EvoState {
  STATE_IDLE,          // sleeping / waiting to listen
  STATE_LISTENING,     // recording user speech
  STATE_PROCESSING,    // uploading STT / waiting
  STATE_THINKING,      // waiting for AI response
  STATE_SPEAKING,      // TTS playing
  STATE_ERROR          // something went wrong
};

EvoState evoState = STATE_IDLE;

// ── Globals ──────────────────────────────────────────────────
String  transcribedText = "";
AiResponse aiResp;
unsigned long stateEnteredAt = 0;

extern bool _ttsDone;

void audio_info(const char *info) {
  Serial.print("[Audio info] "); Serial.println(info);
}
void audio_error(const char *info) {
  Serial.print("[Audio error] "); Serial.println(info);
  _ttsDone = true;
}
void audio_eof_mp3(const char *info) {
  Serial.print("[TTS] EOF: "); Serial.println(info);
  _ttsDone = true;
}

// ── Helpers ──────────────────────────────────────────────────
void enterState(EvoState next) {
  evoState = next;
  stateEnteredAt = millis();

  switch (next) {
    case STATE_IDLE:
      oledEmotion("idle");
      Serial.println("[STATE] IDLE — waiting for touch switch");
      break;

    case STATE_LISTENING:
      oledEmotion("listening");
      Serial.println("[STATE] LISTENING — recording user speech");
      startRecording();
      break;

    case STATE_PROCESSING:
      oledEmotion("thinking");
      Serial.println("[STATE] PROCESSING — uploading to STT");
      break;

    case STATE_THINKING:
      oledEmotion("thinking");
      Serial.println("[STATE] THINKING — waiting for AI");
      break;

    case STATE_SPEAKING:
      oledEmotion(aiResp.animation);
      Serial.println("[STATE] SPEAKING — TTS playback");
      break;

    case STATE_ERROR:
      oledEmotion("confused");
      Serial.println("[STATE] ERROR");
      break;
  }
}

// ── WiFi Setup ───────────────────────────────────────────────
void connectWiFi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WiFi] FAILED — check credentials in config.h");
  }
}

// ── setup() ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== EvoAssistant Booting ===");

  oledInit();
  oledEmotion("idle");

  connectWiFi();

  audioInit();

  pinMode(TOUCH_SWITCH_PIN, INPUT_PULLDOWN);

  Serial.println("=== Evo Ready ===");
  enterState(STATE_IDLE);
}

// ── loop() ──────────────────────────────────────────────────
void loop() {

  // Always tick OLED animations
  oledTick();

  // Always tick TTS audio stream
  audioTick();

  switch (evoState) {

    // ── IDLE: wait for touch switch ────────────────────────
    case STATE_IDLE:
      if (digitalRead(TOUCH_SWITCH_PIN) == HIGH) {
        // Simple debounce
        delay(50);
        if (digitalRead(TOUCH_SWITCH_PIN) == HIGH) {
          playActivationBeep();
          enterState(STATE_LISTENING);
        }
      }
      break;

    // ── LISTENING: record until silence or timeout ─────────
    case STATE_LISTENING:
      if (recordingDone()) {
        stopRecording();
        enterState(STATE_PROCESSING);
      }
      break;

    // ── PROCESSING: run STT ────────────────────────────────
    case STATE_PROCESSING: {
      transcribedText = runSTT();
      Serial.println("[STT] Heard: " + transcribedText);
      if (transcribedText.length() > 0) {
        enterState(STATE_THINKING);
      } else {
        Serial.println("[STT] Nothing heard, going idle");
        enterState(STATE_ERROR);
      }
      break;
    }

    // ── THINKING: call AI ──────────────────────────────────
    case STATE_THINKING: {
      aiResp = askAI(transcribedText);
      Serial.println("[AI] Emotion: " + aiResp.emotion);
      Serial.println("[AI] Text:    " + aiResp.text_response);
      oledEmotion(aiResp.emotion);
      enterState(STATE_SPEAKING);
      break;
    }

    // ── SPEAKING: play TTS, animate mouth ──────────────────
    case STATE_SPEAKING: {
      pauseMic();  // Turn off mic to avoid hardware clock conflict
      
      Serial.println("[STATE] Freeing WiFi sockets...");
      delay(500);  // VERY IMPORTANT: let TCP TIME_WAIT sockets from AI/STT close!
      
      Serial.println("[STATE] Starting TTS...");
      startTTS(aiResp.text_response);
      
      Serial.println("[STATE] Waiting for stream to start...");
      delay(200);
      
      Serial.println("[STATE] Entering audio loop...");
      unsigned long audioStartMs = millis();
      while (isAudioRunning()) {
        audioTick();
        oledTick();
        yield(); // Feed the watchdog timer
        
        // 15 second safety timeout to prevent permanent freezing
        if (millis() - audioStartMs > 15000) {
          Serial.println("[STATE] Audio loop timed out!");
          break;
        }
      }
      
      Serial.println("[STATE] Audio finished!");
      delay(400);
      resumeMic(); // Turn mic back on
      enterState(STATE_IDLE);
      break;
    }

    // ── ERROR: show confused face then recover ─────────────
    case STATE_ERROR:
      if (millis() - stateEnteredAt > 2500) {
        enterState(STATE_IDLE);
      }
      break;
  }
}
