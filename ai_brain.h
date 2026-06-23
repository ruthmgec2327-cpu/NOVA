// ============================================================
//  ai_brain.h + ai_brain.cpp  —  Gemini AI with JSON response
// ============================================================
#pragma once
#include <Arduino.h>

struct AiResponse {
  String emotion        = "idle";
  String text_response  = "";
  String speaker_tone   = "neutral";
  String animation      = "talking";
};

AiResponse askAI(String userText);
