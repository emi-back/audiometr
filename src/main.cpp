#include <Arduino.h>
#include "types.h"
#include "pins.h"
#include "buttons.h"
#include "audio.h"
#include "volume.h"
#include "protocol.h"
#include "display.h"
#include "encoder.h"
#include "manual.h"

DeviceState currentState = STATE_POWER_ON;

volatile AudiometerData audioData;
volatile MenuNavigation menuData; 

int toSendResponses[FREQ_TABLE_SIZE][3][2] = {0};
int finalThresholds[FREQ_TABLE_SIZE][2] = {0};

// Wywoływane przez protocol_hughsonWestlake() za każdym razem, gdy zmienia się głośność,
// żeby ekran SCREEN_2_IN_PROGRESS pokazywał aktualną wartość dB na żywo.
void onIntensityChanged(int db) {
  display_setIntensity(db);
  display_render();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  buttons_init();
  encoder_init();
  display_init();   // SPI + TFT — celowo PRZED I2S/I2C (test diagnostyczny białego ekranu)
  display_showScreen(SCREEN_1_START);

  audio_init();      // I2S + tablica sinusoidy (XSMT trzymane na LOW do końca inicjalizacji)
  volume_init();     // I2C + PT2258
  volume_setDb(&audioData, 20);

  digitalWrite(XSMT_PIN, HIGH); // odciszenie DAC dopiero po pełnej konfiguracji
  Serial.println("System gotowy.");
}

void loop() {
  switch (currentState) {

    case STATE_POWER_ON:
      currentState = STATE_SELF_TEST;
      break;

    case STATE_SELF_TEST:
      // TODO: docelowo faktyczny autotest sprzętu (I2S/I2C/wyświetlacz)
      currentState = STATE_SELECT_MODE;
      break;

    case STATE_SELECT_MODE: {
      // Ekran wyboru: ucho startowe + tryb badania (SCREEN_1_START)
      if (buttonDownPressed) {
        buttonDownPressed = false;
        display_setSelectedRow((display_getSelectedRow() + 1) % 2);
        display_render();
      }
      if (buttonRightPressed) {
        buttonRightPressed = false;
        if (display_getSelectedRow() == 0) display_toggleStartEar();
        else display_toggleMode();
        display_render();
      }
      if (buttonOKPressed) {
        buttonOKPressed = false;

        // UWAGA konwersja konwencji: UI (0=Prawe,1=Lewe) -> logika badania (0=Lewe,1=Prawe)
        // getStartEarIsLeft()==true (Lewe) -> currentEar=false(Lewe); ==false (Prawe) -> currentEar=true(Prawe)
        audioData.currentEar = !display_getStartEarIsLeft();

        audioData.isSemiAutomatic = !display_getModeIsManual();
        currentState = audioData.isSemiAutomatic ? STATE_SEMI_TEST : STATE_MANUAL_TEST;

        display_setMeasurementNum(1);
        display_setFrequency(freqTable[0]);
        display_setIntensity(audioData.intensity);
        display_showScreen(audioData.isSemiAutomatic ? SCREEN_2_IN_PROGRESS : SCREEN_5_MANUAL_FREQ);
      }
      break;
    }

    case STATE_SEMI_TEST: {
      Serial.println("\n=== START TESTU PÓŁAUTOMATYCZNEGO ===");

      // Pętla przechodząca przez OBA ucha (m=0: Lewe ucho, m=1: Prawe ucho)
      for (int m = 0; m < 2; m++) {
        audioData.currentEar = m;

        for (int l = 0; l < FREQ_TABLE_SIZE; l++) {
          protocol_setFrequencyIndex(&audioData, l);

          display_setFrequency(freqTable[l]);
          display_showScreen(SCREEN_2_IN_PROGRESS);

          int responses[3] = {-1, -2, -3};
          int finalThreshold = -1;

          // Pomiar 1
          display_setMeasurementNum(1);
          responses[0] = protocol_hughsonWestlake(&audioData, onIntensityChanged);
          toSendResponses[l][0][m] = responses[0];
          Serial.printf("Pomiar 1: %d dB\n", responses[0]);

          // Pomiar 2
          display_setMeasurementNum(2);
          responses[1] = protocol_hughsonWestlake(&audioData, onIntensityChanged);
          toSendResponses[l][1][m] = responses[1];
          Serial.printf("Pomiar 2: %d dB\n", responses[1]);

          // ZASADA 2 z 2
          if (responses[0] == responses[1]) {
            finalThreshold = responses[0];
            toSendResponses[l][2][m] = responses[0];
            Serial.printf(">> SUKCES (2/2): Próg wynosi %d dB <<\n", finalThreshold);
          } else {
            // Wyniki były różne -> Potrzebny Pomiar 3
            Serial.println("Różne wyniki -> Wykonuję pomiar 3...");
            display_setMeasurementNum(3);
            responses[2] = protocol_hughsonWestlake(&audioData, onIntensityChanged);
            toSendResponses[l][2][m] = responses[2];
            Serial.printf("Pomiar 3: %d dB\n", responses[2]);

            // ZASADA 2 z 3
            if (responses[2] == responses[0] || responses[2] == responses[1]) {
              finalThreshold = responses[2];
              Serial.printf(">> SUKCES (2/3): Próg wynosi %d dB <<\n", finalThreshold);
            } else {
              // Trzy różne wyniki -> szukamy do skutku
              while (true) {
                responses[0] = responses[1];
                responses[1] = responses[2];
                responses[2] = protocol_hughsonWestlake(&audioData, onIntensityChanged);

                toSendResponses[l][0][m] = responses[0];
                toSendResponses[l][1][m] = responses[1];
                toSendResponses[l][2][m] = responses[2];

                if (responses[2] == responses[0] || responses[2] == responses[1]) {
                  finalThreshold = responses[2];
                  Serial.printf(">> SUKCES (po powtórzeniu): Próg wynosi %d dB <<\n", finalThreshold);
                  break;
                }
              }
            }
          }

          finalThresholds[l][m] = finalThreshold;

          // Ekran: pomiar dla tej częstotliwości zakończony, czekamy na OK
          display_showScreen(SCREEN_3_SEMI_FREQ_DONE);

          Serial.println("--> Naciśnij OK, aby przejść do następnej częstotliwości... <--");
          buttonOKPressed = false;
          while (!buttonOKPressed) {
            delay(20);
          }
          buttonOKPressed = false;
        }
      }

      // Dopiero po przetestowaniu OBU uszu przechodzimy do stanu wysyłania danych
      currentState = STATE_FINISHED;
      display_showScreen(SCREEN_4_SUMMARY);
      break;
    }

    case STATE_FINISHED:
      Serial.println("=== TEST ZAKOŃCZONY ===");
      protocol_sendDataToPC(finalThresholds);

      Serial.println("Naciśnij OK, aby wrócić do wyboru trybu...");
      buttonOKPressed = false;
      while (!buttonOKPressed) {
        delay(20);
      }
      buttonOKPressed = false;

      currentState = STATE_SELECT_MODE;
      display_showScreen(SCREEN_1_START);
      break;

    case STATE_MANUAL_TEST:
      manual_runTest(&audioData, onIntensityChanged);
      currentState = STATE_FINISHED;
      display_showScreen(SCREEN_4_SUMMARY);
      break;
  }
}
