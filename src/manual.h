#pragma once
#include "types.h"
#include "protocol.h" // IntensityChangedCallback

// Manualna procedura Hughsona-Westlake'a: operator sam dobiera częstotliwość/natężenie
// (enkodery), sam decyduje kiedy zaprezentować ton (OK) i kiedy uznać bieżące natężenie
// za ostateczną odpowiedź próby (prawy przycisk). Zasada 2 z 3 jak w trybie półautomatycznym.
// Wyniki trafiają do tej samej tablicy toSendResponses co tryb półautomatyczny.
void manual_runTest(volatile AudiometerData *audioData, IntensityChangedCallback onIntensityChanged);
