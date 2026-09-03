#include "protocol.h"
#include "audio.h"
#include "volume.h"
#include "buttons.h"

const uint16_t freqTable[FREQ_TABLE_SIZE] = {1000, 2000, 3000, 4000, 8000, 1000, 250, 500};

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
  volume_setDb(audioData, 50);
  if (onIntensityChanged) onIntensityChanged(audioData->intensity);

  // 1. Ściszanie (-10 dB) gdy pacjent reaguje
  while (true) {
    protocol_wait2to4sec();
    buttonPatientPressed = false;

    audio_generateTone(audioData);
    delay(1000);

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

void protocol_sendDataToPC(int responses[FREQ_TABLE_SIZE][3][2]) {
  Serial.println("\n=======================================================");
  Serial.println("=== WYSYŁANIE KOMPLETU DANYCH (OBA USZY) DO PC ===");
  Serial.println("=======================================================");

  for (int ear = 0; ear < 2; ear++) {
    Serial.printf("\n>>> UCHO %s <<<\n", ear == 0 ? "LEWE" : "PRAWE");
    for (int i = 0; i < FREQ_TABLE_SIZE; i++) {
      Serial.printf("Częstotliwość: %4d Hz | Próba 1: %2d dB | Próba 2: %2d dB | Próba 3: %2d dB\n",
                    freqTable[i],
                    responses[i][0][ear],
                    responses[i][1][ear],
                    responses[i][2][ear]);
    }
  }
  Serial.println("=======================================================\n");
}
