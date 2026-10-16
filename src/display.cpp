#include "display.h"
#include "pins.h"
#include <SPI.h>

// ==========================================
// Parametry wyświetlacza
// ==========================================
#define TFT_W     284
#define TFT_H     76
#define OFFSET_X  18
#define OFFSET_Y  82

#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_GRAY        0x3186
#define COLOR_DARKPURPLE  0x3971
#define COLOR_LIGHTGREEN  0x9FD3
#define COLOR_ORANGE      0xFD20

// ==========================================
// Czcionka 5x7
// ==========================================
static const uint8_t font5x7[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14,
  0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62,
  0x36, 0x49, 0x56, 0x20, 0x50, 0x00, 0x05, 0x03, 0x00, 0x00,
  0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x41, 0x22, 0x1C, 0x00,
  0x14, 0x08, 0x3E, 0x08, 0x14, 0x08, 0x08, 0x3E, 0x08, 0x08,
  0x00, 0x50, 0x30, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x00, 0x60, 0x60, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02,
  0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00,
  0x42, 0x61, 0x51, 0x49, 0x46, 0x21, 0x41, 0x45, 0x4B, 0x31,
  0x18, 0x14, 0x12, 0x7F, 0x10, 0x27, 0x45, 0x45, 0x45, 0x39,
  0x3C, 0x4A, 0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03,
  0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49, 0x29, 0x1E,
  0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x56, 0x36, 0x00, 0x00,
  0x08, 0x14, 0x22, 0x41, 0x00, 0x14, 0x14, 0x14, 0x14, 0x14,
  0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x51, 0x09, 0x06,
  0x32, 0x49, 0x79, 0x41, 0x3E, 0x7E, 0x11, 0x11, 0x11, 0x7E,
  0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E, 0x41, 0x41, 0x41, 0x22,
  0x7F, 0x41, 0x41, 0x22, 0x1C, 0x7F, 0x49, 0x49, 0x49, 0x41,
  0x7F, 0x09, 0x09, 0x01, 0x01, 0x3E, 0x41, 0x49, 0x49, 0x7A,
  0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00,
  0x20, 0x40, 0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41,
  0x7F, 0x40, 0x40, 0x40, 0x40, 0x7F, 0x02, 0x0C, 0x02, 0x7F,
  0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E, 0x41, 0x41, 0x41, 0x3E,
  0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51, 0x21, 0x5E,
  0x7F, 0x09, 0x19, 0x29, 0x46, 0x46, 0x49, 0x49, 0x49, 0x31,
  0x01, 0x01, 0x7F, 0x01, 0x01, 0x3F, 0x40, 0x40, 0x40, 0x3F,
  0x1F, 0x20, 0x40, 0x20, 0x1F, 0x3F, 0x40, 0x38, 0x40, 0x3F,
  0x63, 0x14, 0x08, 0x14, 0x63, 0x07, 0x08, 0x70, 0x08, 0x07,
  0x61, 0x51, 0x49, 0x45, 0x43,
  0x00, 0x7F, 0x41, 0x41, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20,
  0x00, 0x41, 0x41, 0x7F, 0x00, 0x04, 0x02, 0x01, 0x02, 0x04,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x01, 0x02, 0x04, 0x00,
  0x20, 0x54, 0x54, 0x54, 0x78, 0x7F, 0x48, 0x44, 0x44, 0x38,
  0x38, 0x44, 0x44, 0x44, 0x20, 0x38, 0x44, 0x44, 0x48, 0x7F,
  0x38, 0x54, 0x54, 0x54, 0x18, 0x08, 0x7E, 0x09, 0x01, 0x02,
  0x0C, 0x52, 0x52, 0x52, 0x3E, 0x7F, 0x08, 0x04, 0x04, 0x78,
  0x00, 0x44, 0x7D, 0x40, 0x00, 0x20, 0x40, 0x44, 0x3D, 0x00,
  0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00,
  0x7C, 0x04, 0x18, 0x04, 0x78, 0x7C, 0x08, 0x04, 0x04, 0x78,
  0x38, 0x44, 0x44, 0x44, 0x38, 0x7C, 0x14, 0x14, 0x14, 0x08,
  0x08, 0x14, 0x14, 0x18, 0x7C, 0x7C, 0x08, 0x04, 0x04, 0x08,
  0x48, 0x54, 0x54, 0x54, 0x20, 0x04, 0x3F, 0x44, 0x40, 0x20,
  0x3C, 0x40, 0x40, 0x20, 0x7C, 0x1C, 0x20, 0x40, 0x20, 0x1C,
  0x3C, 0x40, 0x30, 0x40, 0x3C, 0x44, 0x28, 0x10, 0x28, 0x44,
  0x0C, 0x50, 0x50, 0x50, 0x3C, 0x44, 0x64, 0x54, 0x4C, 0x44
};

// ==========================================
// Niskopoziomowy sterownik SPI
// ==========================================
static void writeCmd(uint8_t cmd) {
  digitalWrite(TFT_DC_PIN, LOW);
  digitalWrite(TFT_CS_PIN, LOW);
  SPI.transfer(cmd);
  digitalWrite(TFT_CS_PIN, HIGH);
}

static void writeData(uint8_t data) {
  digitalWrite(TFT_DC_PIN, HIGH);
  digitalWrite(TFT_CS_PIN, LOW);
  SPI.transfer(data);
  digitalWrite(TFT_CS_PIN, HIGH);
}

static void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  x0 += OFFSET_X; x1 += OFFSET_X;
  y0 += OFFSET_Y; y1 += OFFSET_Y;

  writeCmd(0x2A);
  writeData(x0 >> 8); writeData(x0 & 0xFF);
  writeData(x1 >> 8); writeData(x1 & 0xFF);

  writeCmd(0x2B);
  writeData(y0 >> 8); writeData(y0 & 0xFF);
  writeData(y1 >> 8); writeData(y1 & 0xFF);

  writeCmd(0x2C);
}

static void display_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  if (x >= TFT_W || y >= TFT_H) return;
  if ((x + w - 1) >= TFT_W) w = TFT_W - x;
  if ((y + h - 1) >= TFT_H) h = TFT_H - y;

  setAddrWindow(x, y, x + w - 1, y + h - 1);
  digitalWrite(TFT_DC_PIN, HIGH);
  digitalWrite(TFT_CS_PIN, LOW);

  uint8_t hi = color >> 8, lo = color & 0xFF;
  uint32_t total = (uint32_t)w * h;
  while (total--) {
    SPI.transfer(hi);
    SPI.transfer(lo);
  }
  digitalWrite(TFT_CS_PIN, HIGH);
}

static void display_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  display_fillRect(x, y, w, 1, color);
  display_fillRect(x, y + h - 1, w, 1, color);
  display_fillRect(x, y, 1, h, color);
  display_fillRect(x + w - 1, y, 1, h, color);
}

static void display_drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
  if (x >= TFT_W || y >= TFT_H) return;
  if (c < 32 || c > 122) c = 32;
  uint16_t index = (c - 32) * 5;

  uint8_t w = 6 * size;
  uint8_t h = 8 * size;

  setAddrWindow(x, y, x + w - 1, y + h - 1);
  digitalWrite(TFT_DC_PIN, HIGH);
  digitalWrite(TFT_CS_PIN, LOW);

  uint8_t hiColor = color >> 8, loColor = color & 0xFF;
  uint8_t hiBg = bg >> 8, loBg = bg & 0xFF;

  for (int8_t row = 0; row < 8; row++) {
    for (uint8_t sy = 0; sy < size; sy++) {
      for (int8_t col = 0; col < 6; col++) {
        bool pixelOn = false;
        if (col < 5) {
          uint8_t line = font5x7[index + col];
          pixelOn = (line >> row) & 0x01;
        }
        for (uint8_t sx = 0; sx < size; sx++) {
          if (pixelOn) {
            SPI.transfer(hiColor);
            SPI.transfer(loColor);
          } else {
            SPI.transfer(hiBg);
            SPI.transfer(loBg);
          }
        }
      }
    }
  }
  digitalWrite(TFT_CS_PIN, HIGH);
}

static uint16_t display_drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size) {
  uint16_t currX = x;
  while (*str) {
    display_drawChar(currX, y, *str, color, bg, size);
    currX += 6 * size;
    str++;
  }
  return currX;
}

// ==========================================
// Stan UI (wewnętrzny, dostęp przez API w display.h)
// ==========================================
static ScreenState currentScreen = SCREEN_1_START;
static bool screenChanged = true;

static int selectedRow = 0;
static int opt_startEar = 0;         // 0: Prawe, 1: Lewe  (konwencja UI, patrz UWAGA w display.h)
static int opt_mode = 0;             // 0: Półauto, 1: Manual
static int opt_nextFreq = 0;         // 0: Tak, 1: Nie
static int opt_hearingThreshold = 0; // 0: Nie, 1: Tak
static int currentFreq = 1000;
static int currentDB = 40;
static int measurementNum = 1;

// --- Tryb manualny ---
static bool manualEarIsLeft = false;
static bool freqLocked = false;
static int lastResponse = -1; // -1: brak tonu, 0: NIE, 1: TAK
static bool manualNoticeIsEarSwitch = false;

// ==========================================
// Renderowanie ekranów
// ==========================================
static void renderScreen1RowsOnly() {
  uint16_t row0Bg = (selectedRow == 0) ? COLOR_DARKPURPLE : COLOR_BLACK;
  display_fillRect(2, 16, 280, 18, row0Bg);
  display_drawRect(2, 16, 280, 18, (selectedRow == 0) ? COLOR_WHITE : COLOR_GRAY);
  display_drawString(6, 21, "Start ucho:", COLOR_WHITE, row0Bg, 1);
  display_drawString(140, 21, "Prawe", (opt_startEar == 0) ? COLOR_YELLOW : COLOR_WHITE, row0Bg, 1);
  display_drawString(185, 21, "|", COLOR_WHITE, row0Bg, 1);
  display_drawString(200, 21, "Lewe", (opt_startEar == 1) ? COLOR_YELLOW : COLOR_WHITE, row0Bg, 1);

  uint16_t row1Bg = (selectedRow == 1) ? COLOR_DARKPURPLE : COLOR_BLACK;
  display_fillRect(2, 36, 280, 18, row1Bg);
  display_drawRect(2, 36, 280, 18, (selectedRow == 1) ? COLOR_WHITE : COLOR_GRAY);
  display_drawString(6, 41, "Tryb badania:", COLOR_WHITE, row1Bg, 1);
  display_drawString(120, 41, "Polauto", (opt_mode == 0) ? COLOR_YELLOW : COLOR_WHITE, row1Bg, 1);
  display_drawString(175, 41, "|", COLOR_WHITE, row1Bg, 1);
  display_drawString(190, 41, "Manual", (opt_mode == 1) ? COLOR_YELLOW : COLOR_WHITE, row1Bg, 1);
}

void display_render() {
  if (screenChanged) {
    display_fillRect(0, 0, TFT_W, TFT_H, COLOR_BLACK);
    screenChanged = false;
  }

  switch (currentScreen) {
    case SCREEN_1_START: {
      display_fillRect(0, 0, 284, 14, COLOR_GRAY);
      display_drawString(6, 3, "Badanie: Wybierz parametry", COLOR_WHITE, COLOR_GRAY, 1);

      renderScreen1RowsOnly();

      display_fillRect(2, 56, 280, 18, COLOR_LIGHTGREEN);
      display_drawString(95, 61, "START BADANIA", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_2_IN_PROGRESS: {
      char buf[48];
      sprintf(buf, "Badanie: w toku (pomiar %d/3)", measurementNum);
      display_drawString(4, 2, buf, COLOR_CYAN, COLOR_BLACK, 1);

      display_fillRect(2, 14, 280, 18, COLOR_GRAY);
      sprintf(buf, "Badane ucho: %s", (opt_startEar == 0) ? "Prawe" : "Lewe");
      display_drawString(6, 19, buf, COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 34, 280, 18, COLOR_GRAY);
      sprintf(buf, "Badana czestotliwosc: %d Hz", currentFreq);
      display_drawString(6, 39, buf, COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 54, 280, 20, COLOR_GRAY);
      sprintf(buf, "Badane natezenie: %d dB HL", currentDB);
      display_drawString(6, 60, buf, COLOR_WHITE, COLOR_GRAY, 1);
      break;
    }

    case SCREEN_3_SEMI_FREQ_DONE: {
      char buf[48];
      sprintf(buf, "Zakonczono pomiar dla %d Hz", currentFreq);
      display_drawString(4, 2, buf, COLOR_WHITE, COLOR_BLACK, 1);

      display_fillRect(2, 16, 280, 26, COLOR_DARKPURPLE);
      display_drawRect(2, 16, 280, 26, COLOR_WHITE);
      display_drawString(6, 24, "Nast. freq?:", COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(190, 24, "TAK", (opt_nextFreq == 0) ? COLOR_YELLOW : COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(220, 24, "|", COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(235, 24, "NIE", (opt_nextFreq == 1) ? COLOR_YELLOW : COLOR_WHITE, COLOR_DARKPURPLE, 1);

      display_fillRect(2, 50, 280, 24, COLOR_LIGHTGREEN);
      display_drawString(95, 58, "WZNOW BADANIE", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_4_SUMMARY: {
      display_drawString(4, 2, "Badanie audiometryczne: zakonczone", COLOR_WHITE, COLOR_BLACK, 1);
      display_fillRect(2, 16, 280, 26, COLOR_GRAY);
      display_drawString(60, 24, "Przeslij dane do aplikacji", COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 50, 280, 24, COLOR_LIGHTGREEN);
      display_drawString(110, 58, "ZATWIERDZ", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_5_MANUAL_FREQ: {
      char buf[48];
      sprintf(buf, "Manualne (proba %d) ucho: %s", measurementNum, manualEarIsLeft ? "Lewe" : "Prawe");
      display_drawString(4, 2, buf, COLOR_CYAN, COLOR_BLACK, 1);

      display_fillRect(2, 14, 280, 18, COLOR_GRAY);
      sprintf(buf, "Czestotliwosc: %d Hz %s", currentFreq, freqLocked ? "[ZABLOKOWANA]" : "[Hz]");
      display_drawString(6, 19, buf, COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 34, 280, 18, COLOR_GRAY);
      sprintf(buf, "Natezenie: %d dB HL [dB]", currentDB);
      display_drawString(6, 39, buf, COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 54, 280, 20, COLOR_GRAY);
      const char* respStr = (lastResponse < 0) ? "--" : (lastResponse == 1 ? "TAK" : "NIE");
      sprintf(buf, "OK=ton PRAWO=zatwierdz  Reakcja: %s", respStr);
      display_drawString(6, 60, buf, COLOR_WHITE, COLOR_GRAY, 1);
      break;
    }

    case SCREEN_6_MANUAL_DB: {
      display_drawString(4, 2, "Badanie manualne: wybierz dB", COLOR_WHITE, COLOR_BLACK, 1);
      display_fillRect(2, 16, 280, 26, COLOR_ORANGE);
      display_drawString(6, 24, "Ustawione natezenie:", COLOR_BLACK, COLOR_ORANGE, 1);
      char buf[16];
      sprintf(buf, "%d dB HL", currentDB);
      display_drawString(180, 24, buf, COLOR_BLACK, COLOR_ORANGE, 1);

      display_fillRect(2, 50, 280, 24, COLOR_LIGHTGREEN);
      display_drawString(110, 58, "ZATWIERDZ", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_7_MANUAL_REACTION: {
      display_drawString(4, 2, "Wykryto reakcje pacjenta!", COLOR_GREEN, COLOR_BLACK, 1);
      display_fillRect(2, 16, 280, 26, COLOR_DARKPURPLE);
      display_drawRect(2, 16, 280, 26, COLOR_WHITE);
      display_drawString(6, 24, "Czy jest to prog slyszenia?:", COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(195, 24, "NIE", (opt_hearingThreshold == 0) ? COLOR_YELLOW : COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(225, 24, "|", COLOR_WHITE, COLOR_DARKPURPLE, 1);
      display_drawString(240, 24, "TAK", (opt_hearingThreshold == 1) ? COLOR_YELLOW : COLOR_WHITE, COLOR_DARKPURPLE, 1);

      display_fillRect(2, 50, 280, 24, COLOR_LIGHTGREEN);
      display_drawString(110, 58, "ZATWIERDZ", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_8_MANUAL_NO_REACTION: {
      display_drawString(4, 2, "Brak reakcji pacjenta", COLOR_RED, COLOR_BLACK, 1);
      display_fillRect(2, 18, 280, 22, COLOR_GRAY);
      display_drawString(30, 24, "Zwieksz natezenie i powtorz", COLOR_WHITE, COLOR_GRAY, 1);

      display_fillRect(2, 48, 280, 26, COLOR_LIGHTGREEN);
      display_drawString(105, 56, "KONTYNUUJ", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }

    case SCREEN_9_MANUAL_STEP_DONE: {
      char buf[48];
      if (manualNoticeIsEarSwitch) {
        display_drawString(4, 2, "Wszystkie czestotliwosci zbadane", COLOR_WHITE, COLOR_BLACK, 1);
        display_fillRect(2, 14, 280, 32, COLOR_GRAY);
        display_drawString(6, 18, "Zmiana ucha.", COLOR_WHITE, COLOR_GRAY, 1);
        display_drawString(6, 30, "OK = kontynuuj", COLOR_WHITE, COLOR_GRAY, 1);
      } else {
        sprintf(buf, "Prog dla %d Hz ustalony", currentFreq);
        display_drawString(4, 2, buf, COLOR_WHITE, COLOR_BLACK, 1);
        display_fillRect(2, 14, 280, 32, COLOR_GRAY);
        sprintf(buf, "Wynik: %d dB HL", currentDB);
        display_drawString(6, 18, buf, COLOR_WHITE, COLOR_GRAY, 1);
        display_drawString(6, 30, "OK = nastepna czestotliwosc", COLOR_WHITE, COLOR_GRAY, 1);
      }

      display_fillRect(2, 50, 280, 24, COLOR_LIGHTGREEN);
      display_drawString(110, 58, "ZATWIERDZ", COLOR_BLACK, COLOR_LIGHTGREEN, 1);
      break;
    }
  }
}

void display_init() {
  pinMode(TFT_CS_PIN,  OUTPUT);
  pinMode(TFT_DC_PIN,  OUTPUT);
  pinMode(TFT_RST_PIN, OUTPUT);
  pinMode(TFT_BL_PIN,  OUTPUT);

  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(TFT_BL_PIN, LOW); 

  SPI.begin(TFT_SCL_PIN, TFT_MISO_PIN, TFT_SDA_PIN, TFT_CS_PIN);
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE3));

  digitalWrite(TFT_RST_PIN, HIGH); delay(10);
  digitalWrite(TFT_RST_PIN, LOW);  delay(50);
  digitalWrite(TFT_RST_PIN, HIGH); delay(150);

  writeCmd(0x11); delay(120);
  writeCmd(0x3A); writeData(0x05);

  writeCmd(0xB2);
  writeData(0x0C); writeData(0x0C); writeData(0x00); writeData(0x33); writeData(0x33);
  writeCmd(0xB0); writeData(0x00); writeData(0xE0);
  writeCmd(0xB7); writeData(0x45);
  writeCmd(0xBB); writeData(0x1D);
  writeCmd(0xC0); writeData(0x2C);
  writeCmd(0xC2); writeData(0x01);
  writeCmd(0xC3); writeData(0x19);
  writeCmd(0xC4); writeData(0x20);
  writeCmd(0xC6); writeData(0x0F);
  writeCmd(0xD0); writeData(0xA4); writeData(0xA1);
  writeCmd(0xD6); writeData(0xA1);

  writeCmd(0x36); writeData(0x70); // Landscape 284x76
  writeCmd(0x20); // Inversion OFF
  delay(10);
  writeCmd(0x29); // Display ON
  delay(120);

  display_fillRect(0, 0, TFT_W, TFT_H, COLOR_BLACK);
}

void display_showScreen(ScreenState screen) {
  currentScreen = screen;
  screenChanged = true;
  display_render();
}

void display_setSelectedRow(int row) { selectedRow = row; }
int  display_getSelectedRow() { return selectedRow; }

void display_toggleStartEar() { opt_startEar = (opt_startEar + 1) % 2; }
void display_toggleMode()     { opt_mode = (opt_mode + 1) % 2; }
bool display_getStartEarIsLeft() { return opt_startEar == 1; }
bool display_getModeIsManual()   { return opt_mode == 1; }

void display_toggleNextFreq()          { opt_nextFreq = (opt_nextFreq + 1) % 2; }
bool display_getNextFreqIsNo()         { return opt_nextFreq == 1; }
void display_toggleHearingThreshold()  { opt_hearingThreshold = (opt_hearingThreshold + 1) % 2; }
bool display_getHearingThresholdIsYes(){ return opt_hearingThreshold == 1; }

void display_setFrequency(int hz) { currentFreq = hz; }
void display_setIntensity(int db) { currentDB = db; }
void display_setMeasurementNum(int n) { measurementNum = n; }

void display_setManualEarIsLeft(bool isLeft) { manualEarIsLeft = isLeft; }
void display_setFreqLocked(bool locked) { freqLocked = locked; }
void display_setLastResponse(int responded) { lastResponse = responded; }
void display_setManualNoticeIsEarSwitch(bool isEarSwitch) { manualNoticeIsEarSwitch = isEarSwitch; }
