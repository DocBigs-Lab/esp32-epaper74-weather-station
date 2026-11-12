#include "textde.h"

// Grundbuchstabe fuer das zweite Byte einer UTF-8-Sequenz 0xC3 xx, 0 = unbekannt
static char umlautBasis(uint8_t b) {
    switch (b) {
        case 0xA4: return 'a';
        case 0xB6: return 'o';
        case 0xBC: return 'u';
        case 0x84: return 'A';
        case 0x96: return 'O';
        case 0x9C: return 'U';
        default:   return 0;
    }
}

static int16_t gradRadius(Adafruit_GFX &g) {
    int16_t x1, y1;
    uint16_t w, h;
    g.getTextBounds("0", 0, 100, &x1, &y1, &w, &h);
    int16_t r = h / 7;
    return r < 2 ? 2 : r;
}

static int16_t gradVorschub(Adafruit_GFX &g) { return 2 * gradRadius(g) + 5; }

int16_t textBreite(Adafruit_GFX &g, const char *s) {
    String ascii;
    int grad = 0;
    for (const uint8_t *p = (const uint8_t *)s; *p;) {
        if (p[0] == 0xC3 && p[1]) {
            if (p[1] == 0x9F) ascii += "ss";
            else if (char b = umlautBasis(p[1])) ascii += b;
            p += 2;
        } else if (p[0] == 0xC2 && p[1] == 0xB0) {
            grad++;
            p += 2;
        } else {
            if (*p < 0x80) ascii += (char)*p;
            p++;
        }
    }
    uint16_t w = 0, h;
    int16_t x1, y1;
    if (ascii.length()) g.getTextBounds(ascii.c_str(), 0, 100, &x1, &y1, &w, &h);
    return w + grad * gradVorschub(g);
}

void textDe(Adafruit_GFX &g, const char *s, uint16_t farbe) {
    g.setTextColor(farbe);
    for (const uint8_t *p = (const uint8_t *)s; *p;) {
        if (p[0] == 0xC3 && p[1]) {
            if (p[1] == 0x9F) {
                g.print("ss");
            } else if (char b = umlautBasis(p[1])) {
                int16_t cx = g.getCursorX(), cy = g.getCursorY();
                char str[2] = {b, 0};
                int16_t bx, by;
                uint16_t bw, bh;
                g.getTextBounds(str, cx, cy, &bx, &by, &bw, &bh);
                g.print(b);
                int16_t d = (bh + 1) / 4;          // Punktgroesse ca. 1/4 der Buchstabenhoehe
                if (d < 2) d = 2;
                int16_t gap = d / 2 < 1 ? 1 : d / 2;
                int16_t y = by - gap - d;
                int16_t xa = bx + bw * 3 / 10 - d / 2, xb = bx + bw * 7 / 10 - d / 2;
                if (d >= 4) {                      // ab 4 px runde Punkte
                    int16_t r = d / 2;
                    g.fillCircle(xa + r, y + r, r, farbe);
                    g.fillCircle(xb + r, y + r, r, farbe);
                } else {
                    g.fillRect(xa, y, d, d, farbe);
                    g.fillRect(xb, y, d, d, farbe);
                }
            }
            p += 2;
        } else if (p[0] == 0xC2 && p[1] == 0xB0) {
            int16_t cx = g.getCursorX(), cy = g.getCursorY();
            int16_t bx, by;
            uint16_t bw, bh;
            g.getTextBounds("0", cx, cy, &bx, &by, &bw, &bh);
            int16_t r = gradRadius(g);
            g.drawCircle(cx + r + 2, by + r, r, farbe);
            if (r >= 3) g.drawCircle(cx + r + 2, by + r, r - 1, farbe);
            g.setCursor(cx + gradVorschub(g), cy);
            p += 2;
        } else {
            if (*p < 0x80) g.print((char)*p);
            p++;
        }
    }
}

void textDe(Adafruit_GFX &g, int16_t x, int16_t y, const char *s, uint16_t farbe, Ausrichtung a) {
    if (a != LINKS) {
        int16_t w = textBreite(g, s);
        x -= (a == MITTE) ? w / 2 : w;
    }
    g.setCursor(x, y);
    textDe(g, s, farbe);
}
