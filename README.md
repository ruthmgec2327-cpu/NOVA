# NOVA

A standalone AI voice assistant built on an **ESP32-S3**. Touch a switch, ask a question out loud, and NOVA records your voice, transcribes it with **Wit.ai**, sends it to **Google Gemini**, and speaks the answer back through a small speaker — all while showing an animated robotic face (eyes + mouth) on a 128×64 OLED.

```
[Touch Switch] → [Record 3s] → [Wit.ai STT] → [Gemini AI] → [Google TTS] → [Speaker]
                                                    │
                                                    ▼
                                          [OLED shows matching emotion]
```
---

## Hardware Required

| Component | Model | Notes |
|---|---|---|
| Microcontroller | ESP32-S3 DevKit | **Must have PSRAM** (used for audio buffers) |
| Microphone | INMP441 | I2S digital MEMS mic |
| Amplifier + Speaker | MAX98357A | I2S mono class-D amp |
| Display | SSD1306 0.96" OLED | 128×64, I2C, addr `0x3C` |
| Trigger | Touch switch (e.g. TTP223) | Digital HIGH output |
| Power | USB 5V or LiPo | 500 mA+ recommended |

---

## Wiring

### INMP441 Microphone → ESP32-S3

```
INMP441 Pin   →   ESP32-S3 GPIO
────────────────────────────────
VDD           →   3.3V
GND           →   GND
WS            →   GPIO 4
SCK           →   GPIO 5
SD            →   GPIO 6
L/R           →   GND   (selects left channel)
```

### MAX98357A Amplifier → ESP32-S3

```
MAX98357A Pin →   ESP32-S3 GPIO
────────────────────────────────
VIN           →   5V (or 3.3V for lower volume)
GND           →   GND
BCLK          →   GPIO 41
LRC           →   GPIO 42
DIN           →   GPIO 40
GAIN          →   floating (15 dB default)
SD            →   floating or 3.3V (always on)
```

### SSD1306 OLED → ESP32-S3

```
OLED Pin      →   ESP32-S3 GPIO
────────────────────────────────
VCC           →   3.3V
GND           →   GND
SDA           →   GPIO 8
SCL           →   GPIO 9
```

### Touch Switch → ESP32-S3

```
Touch Switch Pin → ESP32-S3 GPIO
─────────────────────────────────
OUT              → GPIO 7
VCC              → 3.3V
GND              → GND
```

> All modules share GND. A small breadboard power bus for GND/3.3V keeps wiring tidy. Pin numbers above match the defaults in `config.h` — if you rewire, update that file.

---

## Project Structure

```
NOVA/
├── EvoAssistant.ino     ← setup(), loop(), and the state machine (filename pending rename — see note above)
├── config.h             ← pins, WiFi/API credentials, tunables (EDIT THIS FIRST)
├── oled.h / oled.cpp     ← 9-emotion animated eye/mouth renderer
├── audio_io.h / audio_io.cpp ← I2S mic capture + speaker playback
├── stt.h / stt.cpp       ← Wit.ai speech-to-text upload
├── tts.h / tts.cpp       ← Google Translate TTS streaming playback
├── ai_brain.h / ai_brain.cpp ← Gemini API call + response handling
├── README.md
└── CONTRIBUTING.md
```

---

## How It Works (State Machine)

`EvoAssistant.ino` runs a simple state machine, advanced every `loop()` iteration:

```
STATE_IDLE
   │  touch switch goes HIGH (debounced)
   ▼
STATE_LISTENING        — startRecording() captures 3s of audio into a PSRAM buffer
   │  recordingDone()
   ▼
STATE_PROCESSING       — runSTT() uploads the buffer to Wit.ai, parses transcript
   │  transcript non-empty?
   ├── no  → STATE_ERROR (shows "confused" face for 2.5s, then back to IDLE)
   ▼ yes
STATE_THINKING         — askAI() sends transcript to Gemini, gets back text
   ▼
STATE_SPEAKING         — mic paused, startTTS() streams Google TTS MP3 to speaker,
   │                      OLED shows talking-mouth animation
   ▼
STATE_IDLE             — mic resumed, back to waiting for touch
```

Key implementation details worth knowing:
- **I2S port sharing:** the mic (port 1, RX channel) is explicitly torn down (`pauseMic()`) before TTS playback starts and recreated (`resumeMic()`) afterward, to avoid clock conflicts with the speaker's I2S driver (ESP32-audioI2S).
- **Recording length** is fixed at `RECORD_SECONDS` (3s default) — there's no voice-activity detection or silence trimming.
- A 15-second safety timeout exists in `STATE_SPEAKING` so a stuck audio stream can't hang the device forever.

---

## Required Arduino Libraries

Install via **Arduino IDE → Tools → Manage Libraries**:

| Library | Install Name | Purpose |
|---|---|---|
| Adafruit GFX | `Adafruit GFX Library` | OLED graphics primitives |
| Adafruit SSD1306 | `Adafruit SSD1306` | OLED driver (matches `oled.cpp`) |
| ArduinoJson | `ArduinoJson` | Parsing Gemini's JSON response |
| ESP32-audioI2S | `ESP32-audioI2S` (schreibfaul1) | MP3 streaming over I2S for TTS |

`WiFi`, `WiFiClientSecure`, and `HTTPClient` ship with the ESP32 Arduino core — no separate install needed.

---

## Board Settings (Arduino IDE)

```
Board:            ESP32S3 Dev Module
Upload Speed:     921600
USB Mode:         Hardware CDC and JTAG
Flash Size:       8MB (match your board)
Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
PSRAM:            OPI PSRAM   ← CRITICAL, audio buffers will fail to allocate without this
```

---

## Configuration

All credentials and tunables live in `config.h`:

```cpp
#define WIFI_SSID       "YourNetworkName"
#define WIFI_PASS       "YourPassword"
#define GEMINI_API_KEY  "AIzaSy..."   // from Google AI Studio
#define WIT_AI_TOKEN    "..."          // from wit.ai dashboard
```

### Getting a Gemini API key
1. Visit [Google AI Studio](https://aistudio.google.com).
2. **Get API Key → Create API Key.**
3. Paste into `config.h`.

### Getting a Wit.ai token
1. Visit [wit.ai](https://wit.ai) and sign in.
2. Create a new app → choose your spoken language.
3. **Settings → Server Access Token.**
4. Paste into `config.h`.

---

## Expected Serial Output

```
=== NOVA Booting ===
[OLED] Ready
[WiFi] Connecting to my_wifi
[WiFi] Connected: 192.168.1.42
[Audio] Mic ready (new API)
[Audio] Speaker ready
=== NOVA Ready ===
[STATE] IDLE — waiting for touch switch
[Audio] Recorded 48000 samples
[STATE] PROCESSING — uploading to STT
[STT] Heard: what is the weather like today
[STATE] THINKING — waiting for AI
[AI] Sending: what is the weather like today
[AI] Text: I don't have real-time weather data, but you can check your weather app!
[STATE] SPEAKING — TTS playback
[STATE] Freeing WiFi sockets...
[STATE] Starting TTS...
[STATE] Entering audio loop...
[TTS] EOF: ...
[STATE] Audio finished!
[Audio] Mic channel recreated.
[STATE] IDLE — waiting for touch switch
```

---

## Troubleshooting

| Problem | Likely Cause | Fix |
|---|---|---|
| OLED blank | Wrong I2C pins/address, or wrong library | Confirm SDA=8, SCL=9, addr=0x3C; confirm `Adafruit_SSD1306` is installed (not SH110X) |
| No sound from speaker | Wrong I2S pins or amp not enabled | Check BCLK=41, LRC=42, DIN=40; confirm SD pin floating/3.3V |
| Mic not recording / garbled audio | Wrong I2S pins, or L/R not grounded | Check WS=4, SCK=5, SD=6; confirm INMP441 L/R tied to GND |
| Touch switch does nothing | Wrong pin or wiring | Confirm GPIO 7, `INPUT_PULLDOWN`, switch outputs HIGH when touched |
| STT always empty | Wit.ai token wrong/missing | Re-check `WIT_AI_TOKEN`; confirm app language matches what you're speaking |
| Gemini returns non-200 | API key invalid, billing, or bad model name | Check `GEMINI_API_KEY`; verify the model string in `ai_brain.cpp` is still valid in the Gemini API |
| `ps_malloc` / PSRAM alloc fails | PSRAM not enabled in board settings | Set **PSRAM: OPI PSRAM** in Arduino IDE board menu |
| Device freezes during playback | Audio stream stuck | 15s safety timeout in `STATE_SPEAKING` should recover it; check WiFi stability if it recurs |
| Crash/reboot on boot | Stack overflow from large stack arrays | Audio buffers use `ps_malloc` (PSRAM) — don't add new large arrays on the stack |

---

## Customization

### Change the spoken/recognized language
- In `tts.cpp`, change the language code passed to `connecttospeech()` (e.g. `"en"` → `"hi"`, `"ml"`).
- In the Wit.ai dashboard, set the app's language to match what users will actually speak.

### Add more OLED emotions
1. In `oled.cpp`, write a new `renderXxx(int frame)` function.
2. Wire it into the `if/else` chain in `oledTick()`.
3. Trigger it from `EvoAssistant.ino` or `ai_brain.cpp` wherever you decide it should fire.

### Adjust recording length
Change `RECORD_SECONDS` in `config.h`.

### Adjust mic gain/shift
The `>> 14` bit-shift in `audio_io.cpp`'s `startRecording()` controls how 32-bit I2S samples are scaled down to 16-bit. Adjust if audio is consistently clipped or too quiet.

---

## Power & Performance Notes

- **Response latency** is roughly: 3s recording + ~0.5–1s STT upload + ~1s Gemini call + TTS stream startup.
- **PSRAM** holds the audio recording buffer — never put large (64KB+) buffers on the stack.
- I2S port 1 is used for the mic and is explicitly released/recreated around TTS playback to avoid conflicting with the speaker driver, which also uses I2S internally.

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).
