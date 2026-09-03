#include "audio.h"
#include "pins.h"
#include "driver/i2s.h"
#include <math.h>

static int32_t sineTable[TABLE_SIZE];
static double phase = 0.0;

void audio_init() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    double angle = (2.0 * M_PI * i) / TABLE_SIZE;
    sineTable[i] = (int32_t)(sin(angle) * 2147483647.0);
  }

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = true,
    .tx_desc_auto_clear = true
  };

  i2s_pin_config_t pin_config = {
    .mck_io_num = SCK_PIN,
    .bck_io_num = BCK_PIN,
    .ws_io_num = LCK_PIN,
    .data_out_num = DIN_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  pinMode(XSMT_PIN, OUTPUT);
  digitalWrite(XSMT_PIN, LOW);
  // XSMT ustawiane na HIGH (odciszenie) dopiero po pełnej konfiguracji w setup() głównego programu
}

void audio_generateTone(volatile AudiometerData *audioData) {
  int32_t tx_buffer[BUFFER_SIZE * 2];
  size_t written;

  unsigned long durationMs = random(1000, 2001);
  unsigned long startTime = millis();

  while (millis() - startTime < durationMs) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      int32_t sample = (int32_t)(sineTable[(int)phase]);

      if (audioData->currentEar == false) { // Lewe ucho
        tx_buffer[i * 2]     = sample;
        tx_buffer[i * 2 + 1] = 0;
      } else { // Prawe ucho
        tx_buffer[i * 2]     = 0;
        tx_buffer[i * 2 + 1] = sample;
      }

      phase += audioData->phaseInc;
      if (phase >= TABLE_SIZE) phase -= TABLE_SIZE;
    }

    i2s_write(I2S_NUM_0, tx_buffer, sizeof(tx_buffer), &written, portMAX_DELAY);
  }

  memset(tx_buffer, 0, sizeof(tx_buffer));
  i2s_write(I2S_NUM_0, tx_buffer, sizeof(tx_buffer), &written, portMAX_DELAY);
}

void audio_changeEar(volatile AudiometerData *audioData) {
  audioData->currentEar = !audioData->currentEar;
  Serial.printf("\n======================================\n");
  Serial.printf("ZMIANA UCHA: Teraz badamy %s ucho\n", audioData->currentEar ? "PRAWE" : "LEWE");
  Serial.printf("======================================\n");
}
