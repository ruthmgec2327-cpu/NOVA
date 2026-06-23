// ============================================================
//  config.h  —  All hardware pins, keys & tunable constants
// ============================================================
#pragma once

// ── WiFi ─────────────────────────────────────────────────────
#define WIFI_SSID   "my_wifi"
#define WIFI_PASS   "123456798"

// ── API Keys ─────────────────────────────────────────────────
#define GEMINI_API_KEY   ""   // replace with real key
#define WIT_AI_TOKEN     ""   // replace with real key


// ── I2S Mic (INMP441) ────────────────────────────────────────
#define MIC_WS    4
#define MIC_SCK   5
#define MIC_SD    6

// ── I2S Speaker (MAX98357A) ──────────────────────────────────
#define SPK_BCLK  41
#define SPK_LRC   42
#define SPK_DIN   40

// ── Touch Switch ─────────────────────────────────────────────
#define TOUCH_SWITCH_PIN  7  // Connect touch switch (e.g., TTP223) output here

// ── OLED (SSD1306 0.96") — I2C ───────────────────────────────
#define OLED_SDA   8
#define OLED_SCL   9
#define OLED_ADDR  0x3C
#define OLED_W     128
#define OLED_H     64

// ── Audio ────────────────────────────────────────────────────
#define SAMPLE_RATE       16000
#define RECORD_SECONDS    3
#define VOLUME_BOOST      4
#define NOISE_GATE        100

// ── State Machine ────────────────────────────────────────────
#define WAKE_PAUSE_MS    600      // ms between idle and recording

// ── OLED Animation ──────────────────────────────────────────
#define OLED_TICK_MS     40       // ~25 fps
