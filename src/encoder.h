#pragma once
#include <Arduino.h>

// Konfiguruje piny obu enkoderów (Hz i dB) i podpina przerwania kwadraturowe
void encoder_init();

// Zwraca zakumulowaną liczbę "kliknięć" (dodatnia/ujemna) enkodera od ostatniego odczytu i zeruje licznik
int encoder_readHzDelta();
int encoder_readDbDelta();
