#pragma once
#include "types.h"

// Inicjalizuje I2C i tłumik PT2258 (startowo na db dB tłumienia)
void volume_init();

// Ustawia głośność w dB HL (zakres zależny od częstotliwości, wyznaczony przez
// refAttenuationTable[freqIndex] i zakres tłumienia PT2258 0-79 dB, obcięty do
// wielokrotności 5 dB w stronę zera na obu końcach — żeby kroki po 5/10 dB
// zawsze trafiały w wielokrotność 5). Wynik może być ujemny (poniżej 0 dB HL).
// Przelicza na tłumienie PT2258 i wysyła po I2C.
void volume_setDb(volatile AudiometerData *audioData, int targetDb);

void volume_quiet10db(volatile AudiometerData *audioData);
void volume_louder5db(volatile AudiometerData *audioData);
