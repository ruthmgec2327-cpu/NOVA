// ============================================================
//  stt.cpp — copied exactly from your working standalone code
//  Only difference: reads from shared recBuffer instead of
//  recording its own audio
// ============================================================
#include "stt.h"
#include "config.h"
#include "audio_io.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

String runSTT() {
  int16_t* audioBuffer = getRecordingBuffer();
  int      totalSamples = getRecordingSamples();

  if (!audioBuffer || totalSamples == 0) {
    Serial.println("[STT] No audio buffer");
    return "";
  }

  // Same diagnostics
  int32_t maxV = 0; int64_t sum = 0;
  for (int i = 0; i < totalSamples; i++) {
    int32_t v = abs((int32_t)audioBuffer[i]);
    if (v > maxV) maxV = v;
    sum += v;
  }
  Serial.printf("[STT] max:%d avg:%d\n", maxV, (int)(sum / totalSamples));

  Serial.println("Uploading To Wit.ai...");

  // ── Exact copy of your working standalone code ─────────────
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.begin(client, "https://api.wit.ai/speech?v=20260529");
  https.setTimeout(20000);

  https.addHeader("Authorization", String("Bearer ") + WIT_AI_TOKEN);
  https.addHeader("Content-Type",
    "audio/raw;encoding=signed-integer;bits=16;rate=16000;endian=little");

  int httpCode = https.POST(
    (uint8_t*)audioBuffer,
    totalSamples * sizeof(int16_t)
  );

  String transcript = "";

  if (httpCode > 0) {
    String response = https.getString();

    Serial.println();
    Serial.println("========== RESPONSE ==========");
    Serial.println(response);
    Serial.println("==============================");

    // Extract last non-empty "text" field
    int pos = 0;
    while (pos < (int)response.length()) {
      int idx    = response.indexOf("\"text\"", pos);
      if (idx < 0) break;
      int colon  = response.indexOf(':', idx + 6);
      int qStart = response.indexOf('"', colon + 1);
      int qEnd   = response.indexOf('"', qStart + 1);
      if (colon < 0 || qStart < 0 || qEnd < 0) break;
      String t = response.substring(qStart + 1, qEnd);
      t.trim();
      if (t.length() > 0) transcript = t;
      pos = qEnd + 1;
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  https.end();
  Serial.println("[STT] Heard: '" + transcript + "'");
  return transcript;
}
