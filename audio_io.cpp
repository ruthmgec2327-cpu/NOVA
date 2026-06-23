// ============================================================
//  audio_io.cpp
//  Mic: NEW i2s API (required — ESP32-audioI2S uses new API)
//  Speaker: ESP32-audioI2S owns I2S_NUM_1
//
//  The new API slot config must match legacy behavior:
//  - I2S_DATA_BIT_WIDTH_32BIT input
//  - Left channel only (INMP441 L/R pin = GND)
//  - Same >> 14 shift as working standalone code
// ============================================================
#include "audio_io.h"
#include "config.h"
#include <driver/i2s_std.h>

Audio ttsAudio;

i2s_chan_handle_t rx_chan = NULL;

static bool     recDone    = false;
static int16_t* recBuffer  = nullptr;
static int      recSamples = 0;

static void initMicI2S() {
  i2s_chan_config_t chan_cfg = {
    .id            = I2S_NUM_1,
    .role          = I2S_ROLE_MASTER,
    .dma_desc_num  = 8,
    .dma_frame_num = 64,
    .auto_clear_after_cb = false,
    .auto_clear_before_cb = false,
  };

  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

  i2s_std_config_t std_cfg = {
    .clk_cfg = {
      .sample_rate_hz = (uint32_t)SAMPLE_RATE,
      .clk_src        = I2S_CLK_SRC_DEFAULT,
      .mclk_multiple  = I2S_MCLK_MULTIPLE_256,
    },
    .slot_cfg = {
      .data_bit_width   = I2S_DATA_BIT_WIDTH_32BIT,
      .slot_bit_width   = I2S_SLOT_BIT_WIDTH_AUTO,
      .slot_mode        = I2S_SLOT_MODE_MONO,
      .slot_mask        = I2S_STD_SLOT_LEFT,
      .ws_width         = 32,
      .ws_pol           = false,
      .bit_shift        = true,
      .left_align       = true,
      .big_endian       = false,
      .bit_order_lsb    = false,
    },
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)MIC_SCK,
      .ws   = (gpio_num_t)MIC_WS,
      .dout = I2S_GPIO_UNUSED,
      .din  = (gpio_num_t)MIC_SD,
      .invert_flags = { false, false, false },
    },
  };

  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
  ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

  Serial.println("[Audio] Mic ready (new API)");
}

void audioInit() {
  initMicI2S();
  ttsAudio.setPinout(SPK_BCLK, SPK_LRC, SPK_DIN);
  ttsAudio.setVolume(15);
  Serial.println("[Audio] Speaker ready");
}

void audioTick() { ttsAudio.loop(); }
bool isAudioRunning() { return ttsAudio.isRunning(); }

void playActivationBeep() {
  Serial.println("[Audio] Activation");
}

void startRecording() {
  recSamples = SAMPLE_RATE * RECORD_SECONDS;
  if (recBuffer) { free(recBuffer); recBuffer = nullptr; }
  recBuffer = (int16_t*)ps_malloc(recSamples * sizeof(int16_t));
  if (!recBuffer) {
    Serial.println("[Audio] PSRAM alloc failed!");
    recDone = true; return;
  }
  recDone = false;

  size_t bytesRead;
  const int CHUNK_SIZE = 256;
  int32_t chunkBuffer[CHUNK_SIZE];
  
  for (int i = 0; i < recSamples; i += CHUNK_SIZE) {
    int samplesToRead = min(CHUNK_SIZE, recSamples - i);
    i2s_channel_read(rx_chan, chunkBuffer, samplesToRead * sizeof(int32_t), &bytesRead, portMAX_DELAY);
    
    int samplesRead = bytesRead / sizeof(int32_t);
    for (int j = 0; j < samplesRead; j++) {
      int32_t sample = chunkBuffer[j];
      sample >>= 14;
      if (sample >  32767) sample =  32767;
      if (sample < -32768) sample = -32768;
      recBuffer[i + j] = (int16_t)sample;
    }
    yield(); // Feed the watchdog timer occasionally
  }

  recDone = true;
  Serial.println("[Audio] Recorded " + String(recSamples) + " samples");
}

bool     recordingDone()       { return recDone; }
void     stopRecording()       { recDone = true; }
int16_t* getRecordingBuffer()  { return recBuffer; }
int      getRecordingSamples() { return recSamples; }

static void initMicI2S(); // Forward declaration

void pauseMic() {
  if (rx_chan != NULL) {
    i2s_channel_disable(rx_chan);
    i2s_del_channel(rx_chan);
    rx_chan = NULL;
    Serial.println("[Audio] Mic channel completely freed.");
  }
}

void resumeMic() {
  if (rx_chan == NULL) {
    initMicI2S();
    Serial.println("[Audio] Mic channel recreated.");
  }
}


