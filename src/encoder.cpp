#include "encoder.h"
#include "pins.h"

// Tabela przejść kwadraturowych: indeks = (poprzedni_stan<<2)|obecny_stan (2 bity A,B), wartość = -1/0/+1
static const int8_t quadTable[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

static volatile int16_t hzStep = 0;
static volatile uint8_t hzLastState = 0;
static volatile int16_t dbStep = 0;
static volatile uint8_t dbLastState = 0;

void ARDUINO_ISR_ATTR encoder_hz_callback() {
  uint8_t state = (digitalRead(ENC_HZ_A) << 1) | digitalRead(ENC_HZ_B);
  hzStep += quadTable[(hzLastState << 2) | state];
  hzLastState = state;
}

void ARDUINO_ISR_ATTR encoder_db_callback() {
  uint8_t state = (digitalRead(ENC_DB_A) << 1) | digitalRead(ENC_DB_B);
  dbStep += quadTable[(dbLastState << 2) | state];
  dbLastState = state;
}

void encoder_init() {
  pinMode(ENC_HZ_A, INPUT_PULLUP);
  pinMode(ENC_HZ_B, INPUT_PULLUP);
  pinMode(ENC_DB_A, INPUT_PULLUP);
  pinMode(ENC_DB_B, INPUT_PULLUP);

  hzLastState = (digitalRead(ENC_HZ_A) << 1) | digitalRead(ENC_HZ_B);
  dbLastState = (digitalRead(ENC_DB_A) << 1) | digitalRead(ENC_DB_B);

  attachInterrupt(digitalPinToInterrupt(ENC_HZ_A), encoder_hz_callback, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_HZ_B), encoder_hz_callback, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DB_A), encoder_db_callback, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DB_B), encoder_db_callback, CHANGE);
}


int encoder_readHzDelta() {
  noInterrupts();
  int16_t raw = hzStep;
  hzStep = raw % 4;
  interrupts();
  return raw / 4;
}

int encoder_readDbDelta() {
  noInterrupts();
  int16_t raw = dbStep;
  dbStep = raw % 4;
  interrupts();
  return raw / 4;
}
