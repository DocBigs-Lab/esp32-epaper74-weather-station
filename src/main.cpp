// SPDX-License-Identifier: MIT
// Wetterstation mit Open-Meteo auf dem SES-imagotag 7,4" BWR (Epd74-Treiber)
// Aktualisiert zur vollen und halben Stunde, nachts eine Reinigungsrunde.
// Serieller Monitor (115200): u = sofort aktualisieren, c = Reinigungsrunde
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include <Epd74.h>
#include "ansicht.h"
#include "config.h"
#include "openmeteo.h"
#include "secrets.h"

static Epd74 epd;
static Wetter wetter;
static time_t naechstes = 0;
static int letzteReinigung = -1;   // tm_yday
static bool bildVorhanden = false;

static void zeige(const char *was) {
    Serial.printf("%s ... ", was);
    bool ok = epd.display();
    Serial.printf("%s, %lu ms\n", ok ? "ok" : "BUSY-Timeout", (unsigned long)epd.lastRefreshMs());
}

static void reinigen() {
    epd.fillScreen(EPD_BLACK); zeige("Reinigung schwarz");
    epd.fillScreen(EPD_RED);   zeige("Reinigung rot");
    epd.fillScreen(EPD_WHITE); zeige("Reinigung weiss");
}

static bool wlan() {
    if (WiFi.status() == WL_CONNECTED) return true;
    Serial.print("WLAN verbinden");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print('.');
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? " ok" : " fehlgeschlagen");
    return WiFi.status() == WL_CONNECTED;
}

// true, wenn neue Daten angezeigt wurden
static bool aktualisieren() {
    if (!wlan()) {
        if (!bildVorhanden) { zeichneFehler(epd, "WLAN nicht erreichbar"); zeige("Fehlerbild"); }
        return false;
    }
    struct tm t;
    if (!getLocalTime(&t, 10000)) {
        Serial.println("Keine Uhrzeit (NTP)");
        return false;
    }
    if (t.tm_hour == REINIGUNG_STUNDE && t.tm_yday != letzteReinigung) {
        reinigen();
        letzteReinigung = t.tm_yday;
    }

    String fehler;
    if (!holeWetter(wetter, fehler)) {
        Serial.println("Wetter: " + fehler);
        if (!bildVorhanden) { zeichneFehler(epd, fehler.c_str()); zeige("Fehlerbild"); }
        return false;
    }
    Serial.printf("Wetter: %.1f C, Code %d, %d Stunden, %d Tage\n", wetter.temp, wetter.code, wetter.nh, wetter.nd);
    getLocalTime(&t, 1000);
    zeichneWetter(epd, wetter, t);
    zeige("Wetter");
    bildVorhanden = true;
    return true;
}

static void planeNaechstes(bool ok) {
    time_t jetzt = time(nullptr);
    naechstes = ok ? (jetzt / INTERVALL_S + 1) * INTERVALL_S + 60 : jetzt + RETRY_S;
    struct tm t;
    localtime_r(&naechstes, &t);
    Serial.printf("Naechste Aktualisierung %02d:%02d\n", t.tm_hour, t.tm_min);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Wetterstation " ORT_NAME " ===");
    if (!epd.begin()) {   // Bildpuffer vor WLAN/TLS anlegen
        Serial.println("Kein Speicher fuer die Bildpuffer.");
        while (true) delay(1000);
    }
    epd.setTemperature(PANEL_TEMPERATUR);
    WiFi.setAutoReconnect(true);
    wlan();
    configTzTime(ZEITZONE, NTP_SERVER, "time.nist.gov");
    planeNaechstes(aktualisieren());
}

void loop() {
    if (Serial.available()) {
        switch (Serial.read()) {
            case 'u': planeNaechstes(aktualisieren()); break;
            case 'c': reinigen(); bildVorhanden = false; planeNaechstes(aktualisieren()); break;
            default: break;
        }
    }
    if (time(nullptr) >= naechstes) planeNaechstes(aktualisieren());
    delay(200);
}
