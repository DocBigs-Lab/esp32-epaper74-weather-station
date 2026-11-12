// Text mit deutschen Umlauten und Gradzeichen auf Adafruit-GFX-Fonts.
// Die Fonts kennen nur ASCII; Umlaute werden als Grundbuchstabe + zwei Punkte gezeichnet,
// ß als "ss", ° als kleiner Kreis. Texte im Quelltext als UTF-8.
#pragma once

#include <Adafruit_GFX.h>

enum Ausrichtung { LINKS, MITTE, RECHTS };

// Breite des Textes in Pixeln (aktueller Font)
int16_t textBreite(Adafruit_GFX &g, const char *s);

// Text an der aktuellen Cursorposition zeichnen
void textDe(Adafruit_GFX &g, const char *s, uint16_t farbe);

// Text an Position (x = Bezugspunkt je nach Ausrichtung, y = Grundlinie)
void textDe(Adafruit_GFX &g, int16_t x, int16_t y, const char *s, uint16_t farbe, Ausrichtung a = LINKS);
