#pragma once
#include <Arduino.h>

// Flagi ustawiane w przerwaniach, czyszczone ręcznie po obsłużeniu w loop()
extern volatile bool buttonRightPressed;
extern volatile bool buttonDownPressed;
extern volatile bool buttonOKPressed;
extern volatile bool buttonPatientPressed;

// Konfiguruje piny przycisków i podpina przerwania (FALLING)
void buttons_init();
