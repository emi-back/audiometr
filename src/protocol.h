#pragma once
#include "types.h"

// Losowa przerwa 2-4s między kolejnymi tonami (element protokołu, utrudnia "zgadywanie" przez pacjenta)
void protocol_wait2to4sec();

// Ustawia częstotliwość na podstawie indeksu w freqTable, przelicza inkrement fazy dla generatora
void protocol_setFrequencyIndex(volatile AudiometerData *audioData, int index);

// Typ funkcji zwrotnej wywoływanej za każdym razem, gdy zmienia się głośność w trakcie procedury
// (np. do odświeżenia wyświetlacza na żywo). Może być nullptr.
typedef void (*IntensityChangedCallback)(int currentDb);

// Procedura Hughsona-Westlake'a: ustala próg słyszenia dla aktualnie ustawionej częstotliwości/ucha.
// Zwraca wyznaczony próg w dB HL.
int protocol_hughsonWestlake(volatile AudiometerData *audioData, IntensityChangedCallback onIntensityChanged);

// Wypisuje finalne progi słyszenia (oba uszy) na Serial w formacie CSV, do odbioru przez aplikację na PC
void protocol_sendDataToPC(int finalThresholds[FREQ_TABLE_SIZE][2]);
