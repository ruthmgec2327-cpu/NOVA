// ============================================================
//  oled.h  —  Animated eyes & mouth expression system
// ============================================================
#pragma once
#include <Arduino.h>

void oledInit();
void oledEmotion(String emotion);   // set current emotion
void oledTick();                    // call every loop() iteration
