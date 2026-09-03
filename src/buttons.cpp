#include "buttons.h"
#include "pins.h"

volatile bool buttonRightPressed = false;
volatile bool buttonDownPressed = false;
volatile bool buttonOKPressed = false;
volatile bool buttonPatientPressed = false;

// Przerwania od przycisków (z weryfikacją fizycznego stanu LOW dla uniknięcia zakłóceń)
void ARDUINO_ISR_ATTR button_right_callback() {
  static uint32_t last_interrupt_time = 0;
  uint32_t interrupt_time = millis();
  if ((interrupt_time - last_interrupt_time > 250) && (digitalRead(RIGHT_BUTTON_PIN) == LOW)) {
    buttonRightPressed = true;
    last_interrupt_time = interrupt_time;
  }
}

void ARDUINO_ISR_ATTR button_down_callback() {
  static uint32_t last_interrupt_time = 0;
  uint32_t interrupt_time = millis();
  if ((interrupt_time - last_interrupt_time > 250) && (digitalRead(DOWN_BUTTON_PIN) == LOW)) {
    buttonDownPressed = true;
    last_interrupt_time = interrupt_time;
  }
}

void ARDUINO_ISR_ATTR button_ok_callback() {
  static uint32_t last_interrupt_time = 0;
  uint32_t interrupt_time = millis();
  if ((interrupt_time - last_interrupt_time > 250) && (digitalRead(OK_BUTTON_PIN) == LOW)) {
    buttonOKPressed = true;
    last_interrupt_time = interrupt_time;
  }
}

void ARDUINO_ISR_ATTR button_patient_callback() {
  static uint32_t last_interrupt_time = 0;
  uint32_t interrupt_time = millis();
  if ((interrupt_time - last_interrupt_time > 250) && (digitalRead(PATIENT_BUTTON_PIN) == LOW)) {
    buttonPatientPressed = true;
    last_interrupt_time = interrupt_time;
  }
}

void buttons_init() {
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(OK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PATIENT_BUTTON_PIN, INPUT_PULLUP);

  delay(50);

  attachInterrupt(digitalPinToInterrupt(RIGHT_BUTTON_PIN), button_right_callback, FALLING);
  attachInterrupt(digitalPinToInterrupt(DOWN_BUTTON_PIN), button_down_callback, FALLING);
  attachInterrupt(digitalPinToInterrupt(OK_BUTTON_PIN), button_ok_callback, FALLING);
  attachInterrupt(digitalPinToInterrupt(PATIENT_BUTTON_PIN), button_patient_callback, FALLING);
}
