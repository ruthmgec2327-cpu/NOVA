// ============================================================
//  ai_brain.cpp  —  Calls Gemini, parses structured JSON
// ============================================================
#include "ai_brain.h"
#include "config.h"
#include <WiFi.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

// System prompt for simple chat
static const char* SYSTEM_PROMPT =
  "You are Evo, a friendly robotic voice assistant built on an ESP32-S3. "
  "Answer clearly and concisely in 1 or 2 sentences.";

AiResponse askAI(String userText) {
  AiResponse resp;

  if (WiFi.status() != WL_CONNECTED) {
    resp.text_response = "WiFi is not connected.";
    resp.emotion = "confused";
    return resp;
  }

  HTTPClient http;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.1-flash-lite:generateContent?key=" + String(GEMINI_API_KEY);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(20000);

  // Build simple request body
  DynamicJsonDocument req(2048);
  JsonArray contents = req.createNestedArray("contents");
  JsonObject turn = contents.createNestedObject();
  JsonArray parts = turn.createNestedArray("parts");
  
  // Combine system prompt and user text into a single string
  String combinedText = String(SYSTEM_PROMPT) + "\n\nUser Question: " + userText;
  parts.createNestedObject()["text"] = combinedText;

  String body;
  serializeJson(req, body);

  Serial.println("[AI] Sending: " + userText);
  int code = http.POST(body);

  // Retry once on 429 rate limit
  if (code == 429) {
    Serial.println("[AI] Rate limited, waiting 5s...");
    http.end();
    delay(5000);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    code = http.POST(body);
  }

  if (code == 200) {
    String rawResp = http.getString();
    Serial.println("[AI] Raw: " + rawResp.substring(0, 200));

    DynamicJsonDocument outer(8192);
    DeserializationError err = deserializeJson(outer, rawResp);

    if (!err) {
      String textStr = outer["candidates"][0]["content"]["parts"][0]["text"].as<String>();
      Serial.println("[AI] Text: " + textStr);
      
      textStr.trim();
      resp.text_response = textStr;
      resp.emotion = "talking"; // Default for simple chat
      resp.speaker_tone = "neutral";
      resp.animation = "talking";
    }
  } else {
    Serial.println("[AI] HTTP error: " + String(code));
    resp.text_response = "I had trouble reaching the AI. Error " + String(code);
    resp.emotion = "confused";
  }

  http.end();
  return resp;
}
