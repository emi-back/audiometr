#pragma once
#include <Arduino.h>


enum ScreenState {
  SCREEN_1_START = 1,
  SCREEN_2_IN_PROGRESS,
  SCREEN_3_SEMI_FREQ_DONE,
  SCREEN_4_SUMMARY,
  SCREEN_5_MANUAL_FREQ,
  SCREEN_6_MANUAL_DB,
  SCREEN_7_MANUAL_REACTION,
  SCREEN_8_MANUAL_NO_REACTION,
  SCREEN_9_MANUAL_STEP_DONE
};

// Inicjalizuje SPI i kontroler TFT (na czarno)
void display_init();

// Przełącza na nowy ekran i wymusza pełne przerysowanie
void display_showScreen(ScreenState screen);

// Rysuje bieżący ekran (przy pierwszym wywołaniu po display_showScreen robi pełne czyszczenie,
// kolejne wywołania tylko odświeżają zmienione fragmenty)
void display_render();

// --- Nawigacja / wybór na SCREEN_1_START ---
void display_setSelectedRow(int row);      // 0: ucho, 1: tryb
int  display_getSelectedRow();
void display_toggleStartEar();             // przełącza Prawe/Lewe (konwencja UI: 0=Prawe, 1=Lewe)
void display_toggleMode();                 // przełącza Półauto/Manual
bool display_getStartEarIsLeft();          // true = wybrano Lewe jako startowe
bool display_getModeIsManual();            // true = wybrano tryb manualny

// --- Wybór na SCREEN_3 / SCREEN_7 ---
void display_toggleNextFreq();
bool display_getNextFreqIsNo();
void display_toggleHearingThreshold();
bool display_getHearingThresholdIsYes();

// --- Dane pokazywane w trakcie badania (SCREEN_2, SCREEN_5, SCREEN_6, SCREEN_9...) ---
void display_setFrequency(int hz);
void display_setIntensity(int db);
void display_setMeasurementNum(int n);

// --- Tryb manualny (SCREEN_5, SCREEN_9) ---
void display_setManualEarIsLeft(bool isLeft);
void display_setFreqLocked(bool locked);
void display_setLastResponse(int responded); // -1 = brak tonu, 0 = NIE, 1 = TAK
void display_setManualNoticeIsEarSwitch(bool isEarSwitch);
