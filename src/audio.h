#pragma once
#include "types.h"

// Inicjalizuje tablicę sinusoidy oraz sterownik I2S
void audio_init();

// Generuje ton o losowym czasie trwania (1-2s) do wybranego ucha, potem wycisza wyjście
void audio_generateTone(volatile AudiometerData *audioData);

// Przełącza aktualnie badane ucho i loguje zmianę na Serial
void audio_changeEar(volatile AudiometerData *audioData);
