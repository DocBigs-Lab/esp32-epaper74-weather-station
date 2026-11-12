// Wettersymbole als Vektorgrafik (skalierbar), passend zu den WMO-Codes von Open-Meteo
#pragma once

#include <Adafruit_GFX.h>

// Dicke Linie mit runden Enden
void dickeLinie(Adafruit_GFX &g, float x0, float y0, float x1, float y1, float w, uint16_t farbe);

// Symbol fuer einen WMO-Wettercode, zentriert in einem Quadrat der Kantenlaenge s
void zeichneSymbol(Adafruit_GFX &g, int code, bool tag, int16_t cx, int16_t cy, int16_t s);

// Kurzbeschreibung zum WMO-Code (UTF-8)
const char *wetterText(int code);
