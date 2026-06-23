// ============================================================
//  oled.cpp  —  Robotic animated eye/mouth expressions
//               for SSD1306 0.96" 128×64 OLED
//
//  Emotions: idle, listening, thinking, happy, curious,
//            confused, angry, sleepy, talking
// ============================================================
#include "oled.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SH110X_WHITE SSD1306_WHITE
#define SH110X_BLACK SSD1306_BLACK

Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);

// ── Animation State ─────────────────────────────────────────
static String  currentEmotion = "idle";
static unsigned long lastTick = 0;
static int     animFrame      = 0;

// Eye geometry helpers
struct Eye {
  int cx, cy, w, h;  // center x/y, half-width, half-height
};

static Eye leftEye  = {32, 26, 14, 10};
static Eye rightEye = {96, 26, 14, 10};

// ── Draw a rounded rectangle "eye" ──────────────────────────
void drawEye(Eye e, int squeeze = 0, bool closed = false) {
  int h = closed ? 2 : e.h - squeeze;
  if (h < 1) h = 1;
  display.fillRoundRect(e.cx - e.w, e.cy - h,
                        e.w * 2,    h * 2,
                        min(h, 5), SH110X_WHITE);
}

// ── Draw a small pupil dot ───────────────────────────────────
void drawPupil(Eye e, int dx = 0, int dy = 0) {
  display.fillCircle(e.cx + dx, e.cy + dy, 3, SH110X_BLACK);
}

// ── Mouth helpers ────────────────────────────────────────────
void drawSmile(int level = 0) {
  // level: -2 frown  0 neutral  +2 wide smile
  int y0 = 50, y1 = 50 + level * 3;
  display.drawLine(40, y0,   64, y1,   SH110X_WHITE);
  display.drawLine(64, y1,   88, y0,   SH110X_WHITE);
}

void drawMouthOpen(int openW, int openH) {
  display.fillRoundRect(64 - openW/2, 50, openW, openH, 3, SH110X_WHITE);
  // teeth line
  display.drawLine(64 - openW/2, 52, 64 + openW/2, 52, SH110X_BLACK);
}

void drawMouthLine() {
  display.drawLine(44, 50, 84, 50, SH110X_WHITE);
}

void drawMouthWave(int phase) {
  // Talking mouth: wavy line animated by phase
  int y = 50;
  for (int x = 44; x < 84; x++) {
    int yy = y + (int)(3 * sin((x + phase * 4) * 0.3f));
    display.drawPixel(x, yy, SH110X_WHITE);
    display.drawPixel(x, yy+1, SH110X_WHITE);
  }
}

void drawBlinkLine(Eye e) {
  display.drawLine(e.cx - e.w, e.cy, e.cx + e.w, e.cy, SH110X_WHITE);
}

// ── Individual emotion renderers ─────────────────────────────

void renderIdle(int frame) {
  // Slow breathing blink every ~3s
  bool blinking = (frame % 75 < 4);
  drawEye(leftEye,  0, blinking);
  drawEye(rightEye, 0, blinking);
  if (!blinking) {
    drawPupil(leftEye);
    drawPupil(rightEye);
  }
  // small neutral mouth
  display.drawLine(50, 50, 78, 50, SH110X_WHITE);
}

void renderListening(int frame) {
  // Eyes wide open, pupils shift left/right subtly
  int dx = (frame % 20 < 10) ? -2 : 2;
  drawEye(leftEye);
  drawEye(rightEye);
  drawPupil(leftEye,  dx, 0);
  drawPupil(rightEye, dx, 0);
  // Open mouth O
  int sz = 6 + (frame % 6 < 3 ? 0 : 1);
  drawMouthOpen(sz * 2, sz);
}

void renderThinking(int frame) {
  // One eye squinted, looking up-right
  drawEye(leftEye,  0, false);
  drawEye(rightEye, 4, false);  // squint right
  drawPupil(leftEye,  2, -3);
  drawPupil(rightEye, 3, -3);
  // Mouth zig
  display.drawLine(44, 50, 56, 47, SH110X_WHITE);
  display.drawLine(56, 47, 68, 53, SH110X_WHITE);
  display.drawLine(68, 53, 80, 50, SH110X_WHITE);
  // Rotating dots above
  int dotX = 64 + (int)(16 * cos(frame * 0.2f));
  int dotY = 10 + (int)(6  * sin(frame * 0.2f));
  display.fillCircle(dotX, dotY, 2, SH110X_WHITE);
}

void renderHappy(int frame) {
  bool blink = (frame % 60 < 5);
  if (!blink) {
    // Crescent / happy squint
    drawEye(leftEye,  5, false);
    drawEye(rightEye, 5, false);
    // eyebrow arc up
    display.drawLine(20, 14, 44, 12, SH110X_WHITE);
    display.drawLine(84, 12, 108, 14, SH110X_WHITE);
    drawPupil(leftEye,  0, 2);
    drawPupil(rightEye, 0, 2);
  } else {
    drawBlinkLine(leftEye);
    drawBlinkLine(rightEye);
  }
  // wide smile
  display.drawLine(38, 48, 52, 55, SH110X_WHITE);
  display.drawLine(52, 55, 76, 55, SH110X_WHITE);
  display.drawLine(76, 55, 90, 48, SH110X_WHITE);
}

void renderCurious(int frame) {
  // One eyebrow raised, pupil looking up
  drawEye(leftEye,  0, false);
  drawEye(rightEye, 0, false);
  drawPupil(leftEye,  0, -4);
  drawPupil(rightEye, 0, -4);
  // raised left eyebrow
  display.drawLine(18, 10, 46, 8, SH110X_WHITE);
  // flat right eyebrow
  display.drawLine(82, 12, 110, 12, SH110X_WHITE);
  // small open mouth
  display.drawCircle(64, 50, 4, SH110X_WHITE);
}

void renderConfused(int frame) {
  drawEye(leftEye,  0, false);
  // right eye X
  display.drawLine(84, 18, 108, 34, SH110X_WHITE);
  display.drawLine(108, 18, 84, 34, SH110X_WHITE);
  drawPupil(leftEye, 0, 0);
  // wavy mouth
  display.drawLine(44, 50, 52, 54, SH110X_WHITE);
  display.drawLine(52, 54, 64, 48, SH110X_WHITE);
  display.drawLine(64, 48, 76, 54, SH110X_WHITE);
  display.drawLine(76, 54, 84, 50, SH110X_WHITE);
  // question mark spark
  if (frame % 20 < 10) {
    display.drawChar(110, 6, '?', SH110X_WHITE, SH110X_BLACK, 1);
  }
}

void renderAngry(int frame) {
  drawEye(leftEye,  0, false);
  drawEye(rightEye, 0, false);
  // furrowed brows (diagonal)
  display.drawLine(18, 20, 46, 14, SH110X_WHITE);
  display.drawLine(82, 14, 110, 20, SH110X_WHITE);
  drawPupil(leftEye,  2, 2);
  drawPupil(rightEye,-2, 2);
  // flat frown
  display.drawLine(40, 54, 52, 48, SH110X_WHITE);
  display.drawLine(52, 48, 76, 48, SH110X_WHITE);
  display.drawLine(76, 48, 88, 54, SH110X_WHITE);
  // intensity flicker
  if (frame % 10 < 5) {
    display.drawLine(18, 20, 46, 14, SH110X_WHITE);
  }
}

void renderSleepy(int frame) {
  // Half-closed drooping eyes
  int droop = 4 + (int)(3 * sin(frame * 0.05f));
  drawEye(leftEye,  droop, false);
  drawEye(rightEye, droop, false);
  drawPupil(leftEye,  0, 3);
  drawPupil(rightEye, 0, 3);
  // zzz float upward
  int zz = frame % 40;
  display.setCursor(105, max(0, 20 - zz / 3));
  display.setTextColor(SH110X_WHITE);
  display.print("z");
  if (zz > 10) {
    display.setCursor(110, max(0, 10 - zz / 4));
    display.print("z");
  }
  // tiny yawn mouth
  display.drawCircle(64, 52, 2 + (frame % 30 < 15 ? 0 : 2), SH110X_WHITE);
}

void renderTalking(int frame) {
  drawEye(leftEye,  0, false);
  drawEye(rightEye, 0, false);
  drawPupil(leftEye,  0, 0);
  drawPupil(rightEye, 0, 0);
  // Animated open/close mouth
  int openPhase = frame % 8;
  int h = (openPhase < 4) ? (openPhase * 3 + 4) : ((8 - openPhase) * 3 + 4);
  drawMouthOpen(28, h);
  // eyebrow lift on accent
  if (openPhase == 0) {
    display.drawLine(20, 13, 44, 11, SH110X_WHITE);
    display.drawLine(84, 11, 108, 13, SH110X_WHITE);
  }
}

// ── Public API ───────────────────────────────────────────────

void oledInit() {
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();
  display.display();
  Serial.println("[OLED] Ready");
}

void oledEmotion(String emotion) {
  currentEmotion = emotion;
  animFrame = 0;
}

void oledTick() {
  if (millis() - lastTick < OLED_TICK_MS) return;
  lastTick = millis();
  animFrame++;

  display.clearDisplay();

  if      (currentEmotion == "idle")       renderIdle(animFrame);
  else if (currentEmotion == "listening")  renderListening(animFrame);
  else if (currentEmotion == "thinking")   renderThinking(animFrame);
  else if (currentEmotion == "happy")      renderHappy(animFrame);
  else if (currentEmotion == "curious")    renderCurious(animFrame);
  else if (currentEmotion == "confused")   renderConfused(animFrame);
  else if (currentEmotion == "angry")      renderAngry(animFrame);
  else if (currentEmotion == "sleepy")     renderSleepy(animFrame);
  else if (currentEmotion == "talking")    renderTalking(animFrame);
  else                                     renderIdle(animFrame);

  display.display();
}
