// Bildaufbau der Wetterstation (800 x 480, Querformat)
#pragma once

#include <time.h>

#include <Epd74.h>
#include "openmeteo.h"

void zeichneWetter(Epd74 &epd, const Wetter &w, const struct tm &jetzt);
void zeichneFehler(Epd74 &epd, const char *text);
