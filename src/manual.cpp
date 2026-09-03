#include "manual.h"
#include "audio.h"
#include "volume.h"
#include "buttons.h"
#include "display.h"
#include "encoder.h"

// Zwraca najbliższy (w kierunku direction: +1/-1) indeks w freqTable, dla którego
// freqDone[] jest false, pomijając już zbadane częstotliwości.
static int nextNotDoneIndex(bool freqDone[], int fromIndex, int direction) {
  int idx = fromIndex;
  for (int i = 0; i < FREQ_TABLE_SIZE; i++) {
    idx = (idx + direction + FREQ_TABLE_SIZE) % FREQ_TABLE_SIZE;
    if (!freqDone[idx]) return idx;
  }
  return fromIndex; // nie powinno się zdarzyć — wywołujący sprawdza doneCount wcześniej
}

// Wybiera nową częstotliwość: ustawia fazę generatora (jak protokół półautomatyczny),
// resetuje natężenie do domyślnych 50 dB HL i czyści licznik prób dla tej częstotliwości.
static void selectFrequency(volatile AudiometerData *audioData, int newIndex) {
  protocol_setFrequencyIndex(audioData, newIndex);
  volume_setDb(audioData, 50);
  audioData->responseCount = 0;

  display_setFrequency(freqTable[newIndex]);
  display_setIntensity(audioData->intensity);
  display_setMeasurementNum(1);
  display_setLastResponse(-1);
  display_setFreqLocked(false);
}

// Prezentuje jeden ton przy aktualnym natężeniu (losowy czas trwania jak w trybie
// półautomatycznym) i zapisuje reakcję pacjenta jako wskaźnik na ekranie — nie loguje
// tego jeszcze jako odpowiedzi próby, o tym decyduje operator prawym przyciskiem.
static void presentTone(volatile AudiometerData *audioData) {
  buttonOKPressed = false;
  buttonPatientPressed = false;

  audio_generateTone(audioData);
  delay(1000);

  display_setLastResponse(buttonPatientPressed ? 1 : 0);
  buttonPatientPressed = false;
}

void manual_runTest(volatile AudiometerData *audioData, IntensityChangedCallback onIntensityChanged) {
  Serial.println("\n=== START TESTU MANUALNEGO ===");

  for (int earPass = 0; earPass < 2; earPass++) {
    bool freqDone[FREQ_TABLE_SIZE] = {false};
    int doneCount = 0;

    display_setManualEarIsLeft(audioData->currentEar == false);
    selectFrequency(audioData, 0);
    display_showScreen(SCREEN_5_MANUAL_FREQ);

    while (doneCount < FREQ_TABLE_SIZE) {
      // --- Faza wyboru: enkoder Hz odblokowany, operator dobiera częstotliwość/natężenie ---
      bool locked = false;
      while (!locked) {
        int hzDelta = encoder_readHzDelta();
        if (hzDelta != 0) {
          int newIndex = nextNotDoneIndex(freqDone, audioData->freqIndex, hzDelta > 0 ? 1 : -1);
          selectFrequency(audioData, newIndex);
          display_render();
        }

        int dbDelta = encoder_readDbDelta();
        if (dbDelta != 0) {
          volume_setDb(audioData, audioData->intensity + dbDelta * 5);
          display_setIntensity(audioData->intensity);
          if (onIntensityChanged) onIntensityChanged(audioData->intensity);
        }

        if (buttonOKPressed) {
          display_setFreqLocked(true);
          presentTone(audioData);
          display_render();
          locked = true;
        }

        delay(20);
      }

      // --- Faza zablokowana: zbieranie prób dla bieżącej częstotliwości (zasada 2 z 3) ---
      int finalThreshold = -1;
      while (finalThreshold < 0) {
        int dbDelta = encoder_readDbDelta();
        if (dbDelta != 0) {
          volume_setDb(audioData, audioData->intensity + dbDelta * 5);
          display_setIntensity(audioData->intensity);
          if (onIntensityChanged) onIntensityChanged(audioData->intensity);
        }

        if (buttonOKPressed) {
          presentTone(audioData);
          display_render();
        }

        if (buttonRightPressed) {
          buttonRightPressed = false;

          if (audioData->responseCount < 3) {
            audioData->responses[audioData->responseCount] = audioData->intensity;
            audioData->responseCount++;
          } else {
            // Okno pełne — przesuwamy je (jak procedura półautomatyczna po 3 różnych wynikach)
            audioData->responses[0] = audioData->responses[1];
            audioData->responses[1] = audioData->responses[2];
            audioData->responses[2] = audioData->intensity;
          }

          int rc = audioData->responseCount;
          int f = audioData->freqIndex;
          int ear = audioData->currentEar;

          if (rc == 1) {
            toSendResponses[f][0][ear] = audioData->responses[0];
          } else if (rc == 2) {
            toSendResponses[f][1][ear] = audioData->responses[1];
            if (audioData->responses[0] == audioData->responses[1]) {
              finalThreshold = audioData->responses[0];
              toSendResponses[f][2][ear] = finalThreshold;
            }
          } else { // rc == 3: trzecia próba, lub kolejna po przesunięciu okna
            toSendResponses[f][0][ear] = audioData->responses[0];
            toSendResponses[f][1][ear] = audioData->responses[1];
            toSendResponses[f][2][ear] = audioData->responses[2];
            if (audioData->responses[2] == audioData->responses[0] ||
                audioData->responses[2] == audioData->responses[1]) {
              finalThreshold = audioData->responses[2];
            }
          }

          if (finalThreshold < 0) {
            display_setMeasurementNum(rc < 3 ? rc + 1 : 3);
          }
          display_render();
        }

        delay(20);
      }

      freqDone[audioData->freqIndex] = true;
      doneCount++;

      Serial.printf("Manualnie: %d Hz -> prog %d dB HL\n", freqTable[audioData->freqIndex], finalThreshold);

      display_setIntensity(finalThreshold);
      display_setManualNoticeIsEarSwitch(false);
      display_showScreen(SCREEN_9_MANUAL_STEP_DONE);

      buttonOKPressed = false;
      while (!buttonOKPressed) delay(20);
      buttonOKPressed = false;

      if (doneCount < FREQ_TABLE_SIZE) {
        int nextIndex = nextNotDoneIndex(freqDone, audioData->freqIndex, 1);
        selectFrequency(audioData, nextIndex);
        display_showScreen(SCREEN_5_MANUAL_FREQ);
      }
    }

    if (earPass == 0) {
      display_setManualNoticeIsEarSwitch(true);
      display_showScreen(SCREEN_9_MANUAL_STEP_DONE);

      buttonOKPressed = false;
      while (!buttonOKPressed) delay(20);
      buttonOKPressed = false;

      audioData->currentEar = !audioData->currentEar;
    }
  }

  Serial.println("=== TEST MANUALNY ZAKONCZONY ===");
}
