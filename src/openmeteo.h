// Abfrage von Open-Meteo (https://open-meteo.com), ohne API-Schluessel
#pragma once

#include <Arduino.h>

constexpr int STUNDEN = 48;
constexpr int TAGE = 7;

struct Wetter {
    // jetzt
    float temp = NAN, gefuehlt = NAN;
    int feuchte = 0, code = 0, windRichtung = 0;
    bool tag = true;
    float wind = 0, boeen = 0;

    // naechste 48 Stunden, ab der aktuellen Stunde
    int nh = 0;
    float hTemp[STUNDEN], hRegen[STUNDEN];
    uint8_t hStunde[STUNDEN], hWochentag[STUNDEN];

    // 7 Tage, Index 0 = heute
    int nd = 0;
    int dCode[TAGE], dRisiko[TAGE];
    float dMax[TAGE], dMin[TAGE], dRegen[TAGE], dUv[TAGE];
    char dAufgang[TAGE][6], dUntergang[TAGE][6];
    uint8_t dWochentag[TAGE];
};

// Holt die Daten. Bei Fehler false und Grund in 'fehler'.
bool holeWetter(Wetter &w, String &fehler);
