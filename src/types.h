#pragma once
#include <Arduino.h>


// Stan urządzenia — główna maszyna stanów

typedef enum {
  STATE_POWER_ON,
  STATE_SELF_TEST,
  STATE_SELECT_MODE,
  STATE_SEMI_TEST,
  STATE_MANUAL_TEST,
  STATE_FINISHED
} DeviceState;


// Dane badania (współdzielone między audio/volume/protocol)

struct AudiometerData {
  bool isSemiAutomatic = true;
  bool currentEar = 0;   // UWAGA: 0 = Lewe, 1 = Prawe (konwencja logiki badania)
  int freqIndex = 0;
  int intensity = 20;    // dB
  double phaseInc = 0;
  int responses[3];
  uint8_t responseCount = 0;
};

struct MenuNavigation {
  int currentMenu = 1;
  int currentOption = 1;
  int currentChoice = 1;
  int optionCount = 1;
  int choiceCount = 1;
};


// Tabela częstotliwości testowych

#define FREQ_TABLE_SIZE 8
extern const uint16_t freqTable[FREQ_TABLE_SIZE];

// Tłumienie PT2258 odpowiadające 0 dB HL dla każdej częstotliwości z freqTable
// (kalibracja RETSPL) — ten sam indeks/kolejność co freqTable.
extern const uint8_t refAttenuationTable[FREQ_TABLE_SIZE];

// Wyniki: [częstotliwość][próba 0..2][ucho 0..1]
extern int toSendResponses[FREQ_TABLE_SIZE][3][2];

// Finalne, uzgodnione progi słyszenia: [częstotliwość][ucho 0..1]
extern int finalThresholds[FREQ_TABLE_SIZE][2];


// Parametry audio (I2S)

#define SAMPLE_RATE 44100
#define TABLE_SIZE  1024
#define BUFFER_SIZE 64
