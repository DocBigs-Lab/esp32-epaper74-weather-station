#include "ansicht.h"

#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <math.h>

#include "config.h"
#include "icons.h"
#include "textde.h"

static const char *WT_KURZ[7] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
static const char *WT_LANG[7] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
static const char *MONAT[12] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli",
                                "August", "September", "Oktober", "November", "Dezember"};

// Layout (y-Bereiche)
static constexpr int16_t KOPF_H = 46;
static constexpr int16_t OBEN_Y1 = 198;    // Ende Bereich "Jetzt/Heute"
static constexpr int16_t KURVE_Y1 = 324;   // Ende Verlauf
static constexpr int16_t TAGE_Y1 = 452;    // Ende Tagesvorschau

// ---------- Hilfen -------------------------------------------------------------

// Zahl mit Komma; nachkomma = 0 oder 1. "-0" wird zu "0".
static void zahl(char *buf, size_t n, float v, int nachkomma) {
    if (isnan(v)) {
        snprintf(buf, n, "--");
        return;
    }
    float f = nachkomma ? roundf(v * 10) / 10 : roundf(v);
    if (f == 0) f = 0;
    snprintf(buf, n, nachkomma ? "%.1f" : "%.0f", f);
    for (char *p = buf; *p; p++) if (*p == '.') *p = ',';
}

static void grad(char *buf, size_t n, float v, int nachkomma) {
    char z[16];
    zahl(z, sizeof z, v, nachkomma);
    snprintf(buf, n, "%s°", z);
}

static const char *himmelsrichtung(int g) {
    static const char *r[8] = {"N", "NO", "O", "SO", "S", "SW", "W", "NW"};
    return r[((g % 360) + 360 + 22) / 45 % 8];
}

static void gestrichelt(Adafruit_GFX &g, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t f, int an = 4, int aus = 4) {
    if (y0 == y1) {
        for (int16_t x = x0; x <= x1; x += an + aus) g.drawFastHLine(x, y0, min<int16_t>(an, x1 - x + 1), f);
    } else {
        for (int16_t y = y0; y <= y1; y += an + aus) g.drawFastVLine(x0, y, min<int16_t>(an, y1 - y + 1), f);
    }
}

// ---------- Bereiche -------------------------------------------------------------

static void kopf(Epd74 &e, const struct tm &t) {
    e.fillRect(0, 0, 800, KOPF_H, EPD_RED);
    e.setFont(&FreeSansBold18pt7b);
    textDe(e, 16, 34, ORT_NAME, EPD_WHITE);
    char s[64];
    snprintf(s, sizeof s, "%s, %d. %s %d", WT_LANG[t.tm_wday], t.tm_mday, MONAT[t.tm_mon], t.tm_year + 1900);
    e.setFont(&FreeSans12pt7b);
    textDe(e, 784, 31, s, EPD_WHITE, RECHTS);
}

static void jetzt(Epd74 &e, const Wetter &w) {
    zeichneSymbol(e, w.code, w.tag, 82, 122, 124);
    char s[48];
    e.setFont(&FreeSansBold24pt7b);
    grad(s, sizeof s, w.temp, 1);
    textDe(e, 162, 112, s, w.temp <= 0 ? EPD_RED : EPD_BLACK);
    e.setFont(&FreeSans12pt7b);
    char z[16];
    grad(z, sizeof z, w.gefuehlt, 0);
    snprintf(s, sizeof s, "gefühlt %s", z);
    textDe(e, 164, 148, s, EPD_BLACK);
    textDe(e, 164, 180, wetterText(w.code), EPD_BLACK);
}

// Ein Feld im Raster "Heute": kleine Bezeichnung, darunter der Wert fett
static void feld(Epd74 &e, int16_t x, int16_t y, const char *titel, const char *wert, uint16_t farbe = EPD_BLACK) {
    e.setFont(&FreeSans9pt7b);
    textDe(e, x, y + 16, titel, EPD_BLACK);
    e.setFont(&FreeSansBold12pt7b);
    textDe(e, x, y + 42, wert, farbe);
}

// Raster 2 Spalten x 3 Zeilen rechts oben
static void heute(Epd74 &e, const Wetter &w) {
    const int16_t x1 = 424, x2 = 612, y1 = 52, dy = 48;
    char s[48], a[16], b[16];

    grad(a, sizeof a, w.dMax[0], 0);
    grad(b, sizeof b, w.dMin[0], 0);
    snprintf(s, sizeof s, "%s / %s", a, b);
    feld(e, x1, y1, "Heute max / min", s);

    snprintf(s, sizeof s, "%s - %s", w.dAufgang[0], w.dUntergang[0]);
    feld(e, x2, y1, "Sonne", s);

    zahl(a, sizeof a, w.dRegen[0], 1);
    snprintf(s, sizeof s, "%s mm  %d %%", a, w.dRisiko[0]);
    feld(e, x1, y1 + dy, "Regen, Risiko", s, w.dRisiko[0] >= 70 ? EPD_RED : EPD_BLACK);

    snprintf(s, sizeof s, "%.0f / %.0f km/h %s", w.wind, w.boeen, himmelsrichtung(w.windRichtung));
    feld(e, x2, y1 + dy, "Wind / Böen", s, w.boeen >= 60 ? EPD_RED : EPD_BLACK);

    snprintf(s, sizeof s, "%d %%", w.feuchte);
    feld(e, x1, y1 + 2 * dy, "Luftfeuchte", s);

    zahl(a, sizeof a, w.dUv[0], 0);
    feld(e, x2, y1 + 2 * dy, "UV-Index", a, w.dUv[0] >= 6 ? EPD_RED : EPD_BLACK);
}

// Temperaturkurve (schwarz) und Niederschlag (rote Balken) der naechsten 48 h
static void verlauf(Epd74 &e, const Wetter &w) {
    if (w.nh < 2) return;
    const int16_t px0 = 52, px1 = 784, py0 = OBEN_Y1 + 22, py1 = KURVE_Y1 - 22;
    const float schritt = (float)(px1 - px0) / (w.nh - 1);

    float tmin = 1e9, tmax = -1e9, pmax = 0;
    for (int i = 0; i < w.nh; i++) {
        if (!isnan(w.hTemp[i])) {
            tmin = min(tmin, w.hTemp[i]);
            tmax = max(tmax, w.hTemp[i]);
        }
        pmax = max(pmax, w.hRegen[i]);
    }
    tmin = floorf(tmin) - 1;
    tmax = ceilf(tmax) + 1;
    if (tmax - tmin < 6) {
        float m = (tmax + tmin) / 2;
        tmin = floorf(m - 3);
        tmax = tmin + 6;
    }
    float pskala = max(pmax, 1.0f);   // volle Hoehe = mind. 1 mm/h

    auto xi = [&](int i) { return px0 + i * schritt; };
    auto yt = [&](float t) { return py1 - (t - tmin) / (tmax - tmin) * (py1 - py0); };

    // Regenbalken
    int16_t bw = max<int16_t>(2, (int16_t)schritt - 3);
    for (int i = 0; i < w.nh; i++) {
        if (w.hRegen[i] < 0.05f) continue;
        int16_t h = max<int16_t>(2, lroundf(w.hRegen[i] / pskala * (py1 - py0) * 0.6f));
        e.fillRect(lroundf(xi(i)) - bw / 2, py1 - h, bw, h, EPD_RED);
    }

    // Temperatur-Raster und Beschriftung
    e.setFont(&FreeSans9pt7b);
    float raster = (tmax - tmin) > 16 ? 5 : ((tmax - tmin) > 8 ? 4 : 2);
    for (float t = ceilf(tmin / raster) * raster; t <= tmax; t += raster) {
        int16_t y = lroundf(yt(t));
        gestrichelt(e, px0, y, px1, y, EPD_BLACK, 1, 5);
        char s[12];
        grad(s, sizeof s, t, 0);
        textDe(e, px0 - 8, y + 6, s, EPD_BLACK, RECHTS);
    }
    if (tmin < 0 && tmax > 0) gestrichelt(e, px0, lroundf(yt(0)), px1, lroundf(yt(0)), EPD_RED, 6, 4);

    // Legende Regen rechts oben (Position vorab, damit Tagesnamen nicht hineinragen)
    char z[16], leg[40];
    zahl(z, sizeof z, pmax, 1);
    snprintf(leg, sizeof leg, "Regen bis %s mm/h", z);
    int16_t legX = px1 - textBreite(e, leg) - 18;
    e.fillRect(legX, py0 - 16, 10, 10, EPD_RED);
    textDe(e, px1, py0 - 6, leg, EPD_RED, RECHTS);

    // Tagesgrenzen und Stunden
    textDe(e, px0 + 4, py0 - 6, "Heute", EPD_BLACK);
    for (int i = 0; i < w.nh; i++) {
        int16_t x = lroundf(xi(i));
        if (w.hStunde[i] == 0 && i > 0) {
            gestrichelt(e, x, py0 - 18, x, py1, EPD_BLACK, 3, 3);
            const char *name = WT_LANG[w.hWochentag[i]];
            if (x + 5 + textBreite(e, name) > legX - 10) name = WT_KURZ[w.hWochentag[i]];
            if (x + 5 + textBreite(e, name) <= legX - 10) textDe(e, x + 5, py0 - 6, name, EPD_BLACK);
        }
        if (w.hStunde[i] % 6 == 0) {
            char s[8];
            snprintf(s, sizeof s, "%d Uhr", w.hStunde[i]);
            e.drawFastVLine(x, py1, 4, EPD_BLACK);
            textDe(e, x, py1 + 18, s, EPD_BLACK, MITTE);
        }
    }
    e.drawFastHLine(px0, py1, px1 - px0 + 1, EPD_BLACK);

    // Kurve
    for (int i = 1; i < w.nh; i++) {
        if (isnan(w.hTemp[i - 1]) || isnan(w.hTemp[i])) continue;
        dickeLinie(e, xi(i - 1), yt(w.hTemp[i - 1]), xi(i), yt(w.hTemp[i]), 3, EPD_BLACK);
    }
}

static void tage(Epd74 &e, const Wetter &w) {
    const int anzahl = min(6, w.nd - 1);
    if (anzahl < 1) return;
    const int16_t spalte = 800 / anzahl;
    for (int k = 0; k < anzahl; k++) {
        int d = k + 1;
        int16_t cx = k * spalte + spalte / 2;
        if (k > 0) gestrichelt(e, k * spalte, KURVE_Y1 + 10, k * spalte, TAGE_Y1 - 8, EPD_BLACK, 2, 4);

        e.setFont(&FreeSansBold12pt7b);
        textDe(e, cx, KURVE_Y1 + 26, WT_KURZ[w.dWochentag[d]], w.dWochentag[d] == 0 ? EPD_RED : EPD_BLACK, MITTE);

        zeichneSymbol(e, w.dCode[d], true, cx, KURVE_Y1 + 60, 50);

        // "13° / 5°": max fett, min normal (rot bei Frost)
        char mx[16], mn[16];
        grad(mx, sizeof mx, w.dMax[d], 0);
        grad(mn, sizeof mn, w.dMin[d], 0);
        e.setFont(&FreeSansBold12pt7b);
        int16_t b1 = textBreite(e, mx);
        e.setFont(&FreeSans12pt7b);
        int16_t b2 = textBreite(e, " / "), b3 = textBreite(e, mn);
        int16_t x = cx - (b1 + b2 + b3) / 2;
        e.setFont(&FreeSansBold12pt7b);
        textDe(e, x, KURVE_Y1 + 106, mx, w.dMax[d] <= 0 ? EPD_RED : EPD_BLACK);
        e.setFont(&FreeSans12pt7b);
        textDe(e, " / ", EPD_BLACK);
        textDe(e, mn, w.dMin[d] < 0 ? EPD_RED : EPD_BLACK);

        char z[16], s[32];
        zahl(z, sizeof z, w.dRegen[d], w.dRegen[d] < 10 ? 1 : 0);
        snprintf(s, sizeof s, "%s mm  %d %%", z, w.dRisiko[d]);
        e.setFont(&FreeSans9pt7b);
        textDe(e, cx, KURVE_Y1 + 124, s, w.dRisiko[d] >= 70 ? EPD_RED : EPD_BLACK, MITTE);
    }
}

static void fuss(Epd74 &e, const struct tm &t) {
    e.drawFastHLine(0, TAGE_Y1, 800, EPD_BLACK);
    e.setFont(&FreeSans9pt7b);
    textDe(e, 16, 471, "Daten: Open-Meteo.com", EPD_BLACK);
    char s[32];
    snprintf(s, sizeof s, "aktualisiert %02d:%02d", t.tm_hour, t.tm_min);
    textDe(e, 784, 471, s, EPD_BLACK, RECHTS);
}

// ---------- oeffentlich ------------------------------------------------------------

void zeichneWetter(Epd74 &epd, const Wetter &w, const struct tm &t) {
    epd.fillScreen(EPD_WHITE);
    kopf(epd, t);
    jetzt(epd, w);
    epd.drawFastVLine(400, KOPF_H + 10, OBEN_Y1 - KOPF_H - 18, EPD_BLACK);
    heute(epd, w);
    epd.drawFastHLine(0, OBEN_Y1, 800, EPD_BLACK);
    verlauf(epd, w);
    epd.drawFastHLine(0, KURVE_Y1, 800, EPD_BLACK);
    tage(epd, w);
    fuss(epd, t);
}

void zeichneFehler(Epd74 &epd, const char *text) {
    epd.fillScreen(EPD_WHITE);
    epd.fillRect(0, 0, 800, KOPF_H, EPD_RED);
    epd.setFont(&FreeSansBold18pt7b);
    textDe(epd, 16, 34, ORT_NAME, EPD_WHITE);
    epd.setFont(&FreeSans12pt7b);
    textDe(epd, 400, 230, "Keine Wetterdaten", EPD_BLACK, MITTE);
    textDe(epd, 400, 270, text, EPD_RED, MITTE);
}
