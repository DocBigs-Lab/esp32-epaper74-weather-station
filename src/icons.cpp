#include "icons.h"

#include <math.h>

#include <Epd74.h>

void dickeLinie(Adafruit_GFX &g, float x0, float y0, float x1, float y1, float w, uint16_t farbe) {
    if (w <= 1.5f) {
        g.drawLine(lroundf(x0), lroundf(y0), lroundf(x1), lroundf(y1), farbe);
        return;
    }
    float dx = x1 - x0, dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.01f) {
        g.fillCircle(lroundf(x0), lroundf(y0), lroundf(w / 2), farbe);
        return;
    }
    float nx = -dy / len * w / 2, ny = dx / len * w / 2;
    g.fillTriangle(lroundf(x0 + nx), lroundf(y0 + ny), lroundf(x1 + nx), lroundf(y1 + ny),
                   lroundf(x1 - nx), lroundf(y1 - ny), farbe);
    g.fillTriangle(lroundf(x0 + nx), lroundf(y0 + ny), lroundf(x1 - nx), lroundf(y1 - ny),
                   lroundf(x0 - nx), lroundf(y0 - ny), farbe);
    int16_t r = lroundf(w / 2 - 0.5f);
    g.fillCircle(lroundf(x0), lroundf(y0), r, farbe);
    g.fillCircle(lroundf(x1), lroundf(y1), r, farbe);
}

// ---------- Bausteine ---------------------------------------------------------

static void sonne(Adafruit_GFX &g, float x, float y, float r) {
    float w = r / 4.5f < 2 ? 2 : r / 4.5f;
    for (int i = 0; i < 8; i++) {
        float a = i * (float)M_PI / 4;
        dickeLinie(g, x + cosf(a) * r * 1.38f, y + sinf(a) * r * 1.38f,
                   x + cosf(a) * r * 1.8f, y + sinf(a) * r * 1.8f, w, EPD_RED);
    }
    g.fillCircle(lroundf(x), lroundf(y), lroundf(r), EPD_RED);
}

static void mond(Adafruit_GFX &g, float x, float y, float r) {
    g.fillCircle(lroundf(x), lroundf(y), lroundf(r), EPD_BLACK);
    g.fillCircle(lroundf(x + r * 0.45f), lroundf(y - r * 0.3f), lroundf(r * 0.85f), EPD_WHITE);
}

static void himmel(Adafruit_GFX &g, bool tag, float x, float y, float r) {
    if (tag) sonne(g, x, y, r);
    else mond(g, x, y, r * 1.2f);
}

// Wolke: drei Kreise auf einem Sockel. Zuerst schwarz (Umriss), dann innen in 'innen'.
static void wolke(Adafruit_GFX &g, float cx, float cy, float w, float t, uint16_t innen) {
    const float k[3][3] = {{-0.25f, 0.05f, 0.20f}, {0.02f, -0.10f, 0.27f}, {0.27f, 0.05f, 0.20f}};
    for (int pass = 0; pass < 2; pass++) {
        float d = pass == 0 ? t : 0;
        uint16_t f = pass == 0 ? EPD_BLACK : innen;
        if (pass == 1 && innen == EPD_BLACK) break;
        for (auto &c : k) g.fillCircle(lroundf(cx + c[0] * w), lroundf(cy + c[1] * w), lroundf(c[2] * w + d), f);
        float x0 = cx - 0.25f * w - d, x1 = cx + 0.27f * w + d;
        float y0 = cy + 0.05f * w - d, y1 = cy + 0.25f * w + d;
        g.fillRect(lroundf(x0), lroundf(y0), lroundf(x1 - x0), lroundf(y1 - y0), f);
    }
}

static void regen(Adafruit_GFX &g, float cx, float y, float s, uint16_t farbe) {
    float w = s / 28 < 2 ? 2 : s / 28;
    for (int k = -1; k <= 1; k++) {
        float x = cx + k * 0.2f * s;
        dickeLinie(g, x + 0.05f * s, y, x - 0.05f * s, y + 0.2f * s, w, farbe);
    }
}

static void niesel(Adafruit_GFX &g, float cx, float y, float s) {
    int16_t r = s / 32 < 2 ? 2 : s / 32;
    for (int k = -1; k <= 1; k++) {
        g.fillCircle(lroundf(cx + k * 0.2f * s), lroundf(y + 0.04f * s), r, EPD_BLACK);
        g.fillCircle(lroundf(cx + k * 0.2f * s - 0.08f * s), lroundf(y + 0.17f * s), r, EPD_BLACK);
    }
}

static void flocke(Adafruit_GFX &g, float x, float y, float r, float w) {
    for (int i = 0; i < 3; i++) {
        float a = i * (float)M_PI / 3 + (float)M_PI / 2;
        dickeLinie(g, x - cosf(a) * r, y - sinf(a) * r, x + cosf(a) * r, y + sinf(a) * r, w, EPD_BLACK);
    }
}

static void schnee(Adafruit_GFX &g, float cx, float y, float s) {
    float r = 0.07f * s, w = s / 45 < 1.5f ? 1.5f : s / 45;
    flocke(g, cx - 0.22f * s, y + 0.06f * s, r, w);
    flocke(g, cx, y + 0.15f * s, r, w);
    flocke(g, cx + 0.22f * s, y + 0.06f * s, r, w);
}

static void blitz(Adafruit_GFX &g, float cx, float y, float s) {
    g.fillTriangle(lroundf(cx + 0.07f * s), lroundf(y), lroundf(cx - 0.09f * s), lroundf(y + 0.17f * s),
                   lroundf(cx + 0.03f * s), lroundf(y + 0.17f * s), EPD_RED);
    g.fillTriangle(lroundf(cx - 0.03f * s), lroundf(y + 0.13f * s), lroundf(cx + 0.09f * s), lroundf(y + 0.13f * s),
                   lroundf(cx - 0.07f * s), lroundf(y + 0.33f * s), EPD_RED);
}

// ---------- Symbole -------------------------------------------------------------

void zeichneSymbol(Adafruit_GFX &g, int code, bool tag, int16_t cx, int16_t cy, int16_t s) {
    float t = s / 35.0f < 2 ? 2 : s / 35.0f;
    switch (code) {
        case 0:   // klar
            himmel(g, tag, cx, cy, 0.22f * s);
            break;
        case 1:   // ueberwiegend klar
            himmel(g, tag, cx - 0.12f * s, cy - 0.12f * s, 0.2f * s);
            wolke(g, cx + 0.12f * s, cy + 0.12f * s, 0.55f * s, t, EPD_WHITE);
            break;
        case 2:   // teils bewoelkt
            himmel(g, tag, cx - 0.18f * s, cy - 0.18f * s, 0.17f * s);
            wolke(g, cx + 0.03f * s, cy + 0.05f * s, 0.8f * s, t, EPD_WHITE);
            break;
        case 3:   // bedeckt
            wolke(g, cx + 0.15f * s, cy - 0.12f * s, 0.55f * s, t, EPD_BLACK);
            wolke(g, cx - 0.05f * s, cy + 0.06f * s, 0.8f * s, t, EPD_WHITE);
            break;
        case 45: case 48: {   // Nebel
            wolke(g, cx, cy - 0.15f * s, 0.7f * s, t, EPD_WHITE);
            float w = s / 25 < 2 ? 2 : s / 25;
            dickeLinie(g, cx - 0.32f * s, cy + 0.14f * s, cx + 0.22f * s, cy + 0.14f * s, w, EPD_BLACK);
            dickeLinie(g, cx - 0.22f * s, cy + 0.25f * s, cx + 0.32f * s, cy + 0.25f * s, w, EPD_BLACK);
            dickeLinie(g, cx - 0.32f * s, cy + 0.36f * s, cx + 0.22f * s, cy + 0.36f * s, w, EPD_BLACK);
            break;
        }
        case 51: case 53: case 55:   // Niesel
            wolke(g, cx, cy - 0.12f * s, 0.8f * s, t, EPD_WHITE);
            niesel(g, cx, cy + 0.16f * s, s);
            break;
        case 56: case 57: case 66: case 67:   // gefrierend
            wolke(g, cx, cy - 0.12f * s, 0.8f * s, t, EPD_WHITE);
            regen(g, cx, cy + 0.16f * s, s, EPD_RED);
            break;
        case 61: case 63: case 65:   // Regen
            wolke(g, cx, cy - 0.12f * s, 0.8f * s, t, EPD_WHITE);
            regen(g, cx, cy + 0.16f * s, s, EPD_BLACK);
            break;
        case 71: case 73: case 75: case 77: case 85: case 86:   // Schnee
            wolke(g, cx, cy - 0.12f * s, 0.8f * s, t, EPD_WHITE);
            schnee(g, cx, cy + 0.16f * s, s);
            break;
        case 80: case 81: case 82:   // Schauer
            himmel(g, tag, cx - 0.2f * s, cy - 0.24f * s, 0.15f * s);
            wolke(g, cx + 0.02f * s, cy - 0.1f * s, 0.78f * s, t, EPD_WHITE);
            regen(g, cx, cy + 0.17f * s, s, EPD_BLACK);
            break;
        case 95: case 96: case 99:   // Gewitter
            wolke(g, cx, cy - 0.12f * s, 0.8f * s, t, EPD_WHITE);
            blitz(g, cx, cy + 0.1f * s, s);
            break;
        default:
            wolke(g, cx, cy, 0.8f * s, t, EPD_WHITE);
            break;
    }
}

const char *wetterText(int code) {
    switch (code) {
        case 0:  return "Klar";
        case 1:  return "Überwiegend klar";
        case 2:  return "Teils bewölkt";
        case 3:  return "Bedeckt";
        case 45: return "Nebel";
        case 48: return "Reifnebel";
        case 51: return "Leichter Niesel";
        case 53: return "Niesel";
        case 55: return "Starker Niesel";
        case 56: case 57: return "Gefrierender Niesel";
        case 61: return "Leichter Regen";
        case 63: return "Regen";
        case 65: return "Starker Regen";
        case 66: case 67: return "Gefrierender Regen";
        case 71: return "Leichter Schneefall";
        case 73: return "Schneefall";
        case 75: return "Starker Schneefall";
        case 77: return "Schneegriesel";
        case 80: return "Leichte Schauer";
        case 81: return "Regenschauer";
        case 82: return "Heftige Schauer";
        case 85: return "Schneeschauer";
        case 86: return "Starke Schneeschauer";
        case 95: return "Gewitter";
        case 96: case 99: return "Gewitter mit Hagel";
        default: return "Unbekannt";
    }
}
