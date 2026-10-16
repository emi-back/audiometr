#include "volume.h"
#include "pins.h"
#include <Wire.h>

static void pt2258_set_channel_attenuation(uint8_t channel, uint8_t db) {
  if (db > 79) db = 79;

  uint8_t tens = db / 10;
  uint8_t units = db % 10;

  uint8_t tens_cmd = 0;
  uint8_t units_cmd = 0;

  switch (channel) {
    case 1:
      tens_cmd  = 0x80 | tens;
      units_cmd = 0x90 | units;
      break;
    case 2:
      tens_cmd  = 0x40 | tens;
      units_cmd = 0x50 | units;
      break;
    case 3:
      tens_cmd  = 0x00 | tens;
      units_cmd = 0x10 | units;
      break;
    case 4:
      tens_cmd  = 0x20 | tens;
      units_cmd = 0x30 | units;
      break;
    case 5:
      tens_cmd  = 0x60 | tens;
      units_cmd = 0x70 | units;
      break;
    case 6:
      tens_cmd  = 0xA0 | tens;
      units_cmd = 0xB0 | units;
      break;
    default:
      return;
  }

  Wire.beginTransmission(PT2258_ADDR);
  Wire.write(tens_cmd);
  Wire.write(units_cmd);
  Wire.endTransmission();
}

static void pt2258_init_exact(uint8_t db) {
  delay(300);

  Wire.beginTransmission(PT2258_ADDR);
  Wire.write(0xC0);
  Wire.endTransmission();
  delay(50);

  Wire.beginTransmission(PT2258_ADDR);
  Wire.write(0xF8);
  Wire.endTransmission();
  delay(50);

  for (uint8_t ch = 1; ch <= 6; ch++) {
    pt2258_set_channel_attenuation(ch, db);
  }
}

void volume_init() {
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(50000);
  Wire.setTimeOut(1000);

  pt2258_init_exact(50);
}

void volume_setDb(volatile AudiometerData *audioData, int targetDb) {
  int rawMaxDb = refAttenuationTable[audioData->freqIndex];
  // Granice obcinamy do wielokrotności 5 dB w stronę zera, nawet kosztem zakresu na obu końcach — żeby
  // kroki po 5/10 dB zawsze trafiały w wielokrotność 5.
  int maxDb = (rawMaxDb / 5) * 5;
  int minDb = ((maxDb - 79) / 5) * 5;

  if (targetDb < minDb) targetDb = minDb;
  if (targetDb > maxDb) targetDb = maxDb;

  audioData->intensity = targetDb;
  uint8_t attenuation = maxDb - targetDb;

  pt2258_set_channel_attenuation(1, attenuation);
  pt2258_set_channel_attenuation(2, attenuation);
}

void volume_quiet10db(volatile AudiometerData *audioData) {
  volume_setDb(audioData, audioData->intensity - 10);
}

void volume_louder5db(volatile AudiometerData *audioData) {
  volume_setDb(audioData, audioData->intensity + 5);
}
