#pragma once
#include "types.h"

// Inicjalizuje I2C i tłumik PT2258 (startowo na db dB tłumienia)
void volume_init();

// Ustawia głośność w dB HL (0-70), przelicza na tłumienie PT2258 i wysyła po I2C
void volume_setDb(volatile AudiometerData *audioData, int targetDb);

void volume_quiet10db(volatile AudiometerData *audioData);
void volume_louder5db(volatile AudiometerData *audioData);
