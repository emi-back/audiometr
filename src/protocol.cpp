#include "protocol.h"
#include "audio.h"
#include "volume.h"
#include "buttons.h"

const uint16_t freqTable[FREQ_TABLE_SIZE] = {1000, 2000, 3000, 4000, 8000, 1000, 250, 500};
const uint8_t refAttenuationTable[FREQ_TABLE_SIZE] = {67, 73, 78, 77, 56, 67, 55, 64};

void protocol_wait2to4sec() {
  unsigned long durationMs = random(2000, 4001);
  delay(durationMs);
}

void protocol_setFrequencyIndex(volatile AudiometerData *audioData, int index) {
  audioData->freqIndex = index % FREQ_TABLE_SIZE;
  audioData->phaseInc = (TABLE_SIZE * (double)freqTable[audioData->freqIndex]) / SAMPLE_RATE;
  Serial.printf("\n--- Ustawiono częstotliwość: %d Hz ---\n", freqTable[audioData->freqIndex]);
}

int protocol_hughsonWestlake(volatile AudiometerData *audioData, IntensityChangedCallback onIntensityChanged) {
  volume_setDb(audioData, 30);
  if (onIntensityChanged) onIntensityChanged(audioData->intensity);

  // 1. Ściszanie (-10 dB) gdy pacjent reaguje
  while (true) {
    protocol_wait2to4sec();
    buttonPatientPressed = false;

    audio_generateTone(audioData);
    delay(400);

    if (buttonPatientPressed) {
      buttonPatientPressed = false;
      volume_quiet10db(audioData);
      if (onIntensityChanged) onIntensityChanged(audioData->intensity);
    } else {
      break;
    }
  }

  // 2. Podgłaśnianie (+5 dB) do wyznaczenia progu
  while (true) {
    volume_louder5db(audioData);
    if (onIntensityChanged) onIntensityChanged(audioData->intensity);
    protocol_wait2to4sec();
    buttonPatientPressed = false;

    audio_generateTone(audioData);
    delay(1000);

    if (buttonPatientPressed) {
      buttonPatientPressed = false;
      return audioData->intensity;
    }
  }
}

void protocol_sendDataToPC(int finalThresholds[FREQ_TABLE_SIZE][2]) {
  Serial.println("===CSV_START===");
  Serial.println("ear,frequency_hz,threshold_db");
  for (int ear = 0; ear < 2; ear++) {
    const char *earLabel = (ear == 0) ? "L" : "P";
    for (int i = 0; i < FREQ_TABLE_SIZE; i++) {
      Serial.printf("%s,%d,%d\n", earLabel, freqTable[i], finalThresholds[i][ear]);
    }
  }
  Serial.println("===CSV_END===");
}
