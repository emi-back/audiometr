#pragma once
#include <Arduino.h>

// ==========================================
// I2S (DAC audio) — z main.cpp
// ==========================================
#define LCK_PIN (digitalPinToGPIONumber(D7))
#define DIN_PIN (digitalPinToGPIONumber(D8))
#define BCK_PIN (digitalPinToGPIONumber(D9))
#define SCK_PIN (digitalPinToGPIONumber(D10))

// Sterowanie wyciszeniem DAC (Soft Mute)
#define XSMT_PIN D4

// ==========================================
// I2C (regulacja głośności PT2258) — z main.cpp
// ==========================================
#define PT2258_ADDR 0x46
#define I2C_SDA     D5
#define I2C_SCL     D6

// ==========================================
// Przyciski — wspólne dla logiki badania i UI
// (te same piny w main.cpp i display.cpp, tylko inaczej nazwane)
// ==========================================
#define RIGHT_BUTTON_PIN   D0   // Zmiana opcji / częstotliwości
#define DOWN_BUTTON_PIN    D2   // Zmiana wiersza / głośności
#define OK_BUTTON_PIN      D1   // Potwierdzenie wyboru
#define PATIENT_BUTTON_PIN D3   // Odpowiedź pacjenta (nieużywany w display.cpp)

// ==========================================
// Wyświetlacz TFT (SPI) — z display.cpp
// ==========================================
#define TFT_SCL_PIN   D13
#define TFT_SDA_PIN   D11
#define TFT_MISO_PIN  D12   // wymagany przez API SPI, fizycznie niepodłączony
#define TFT_CS_PIN    A0
#define TFT_DC_PIN    A7
#define TFT_RST_PIN   A6
#define TFT_BL_PIN    A1

// ==========================================
// Enkodery (tryb manualny) — Hz: A2/A3, dB: A4/A5
// ==========================================
#define ENC_HZ_A    A3
#define ENC_HZ_B    A2
#define ENC_DB_A    A5
#define ENC_DB_B    A4
