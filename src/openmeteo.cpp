#include "openmeteo.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

#include "config.h"

static const char *URL =
    "http://api.open-meteo.com/v1/forecast?latitude=" ORT_LAT "&longitude=" ORT_LON
    "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,"
    "wind_speed_10m,wind_direction_10m,wind_gusts_10m,is_day"
    "&hourly=temperature_2m,precipitation"
    "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_sum,"
    "precipitation_probability_max,sunrise,sunset,uv_index_max"
    "&timezone=Europe%2FBerlin&forecast_days=7&forecast_hours=48";

// Wochentag (0 = Sonntag) aus "JJJJ-MM-TT..."
static uint8_t wochentag(const char *iso) {
    struct tm t = {};
    if (sscanf(iso, "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday) != 3) return 0;
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    t.tm_hour = 12;
    mktime(&t);
    return t.tm_wday;
}

// "HH:MM" aus "JJJJ-MM-TTTHH:MM"
static void uhrzeit(const char *iso, char out[6]) {
    if (iso && strlen(iso) >= 16) {
        memcpy(out, iso + 11, 5);
        out[5] = 0;
    } else {
        strcpy(out, "--:--");
    }
}

bool holeWetter(Wetter &w, String &fehler) {
    // Einfaches HTTP: die installierte ESP32-Plattform bringt keine TLS-Bibliothek mit,
    // und es werden nur oeffentliche Wetterdaten abgerufen.
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(15000);
    if (!http.begin(client, URL)) {
        fehler = "HTTP-Start fehlgeschlagen";
        return false;
    }
    int status = http.GET();
    if (status != 200) {
        fehler = "HTTP " + String(status);
        http.end();
        return false;
    }
    String body = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError e = deserializeJson(doc, body);
    if (e) {
        fehler = String("JSON: ") + e.c_str();
        return false;
    }

    JsonObject c = doc["current"];
    w.temp = c["temperature_2m"] | NAN;
    w.gefuehlt = c["apparent_temperature"] | NAN;
    w.feuchte = c["relative_humidity_2m"] | 0;
    w.code = c["weather_code"] | 0;
    w.wind = c["wind_speed_10m"] | 0.0f;
    w.boeen = c["wind_gusts_10m"] | 0.0f;
    w.windRichtung = c["wind_direction_10m"] | 0;
    w.tag = (c["is_day"] | 1) == 1;

    JsonObject h = doc["hourly"];
    JsonArray hz = h["time"], ht = h["temperature_2m"], hr = h["precipitation"];
    w.nh = 0;
    for (size_t i = 0; i < hz.size() && i < STUNDEN; i++) {
        const char *iso = hz[i];
        w.hTemp[i] = ht[i] | NAN;
        w.hRegen[i] = hr[i] | 0.0f;
        w.hStunde[i] = (iso && strlen(iso) >= 13) ? atoi(iso + 11) : 0;
        w.hWochentag[i] = iso ? wochentag(iso) : 0;
        w.nh++;
    }

    JsonObject d = doc["daily"];
    JsonArray dz = d["time"];
    w.nd = 0;
    for (size_t i = 0; i < dz.size() && i < TAGE; i++) {
        w.dWochentag[i] = wochentag(dz[i] | "");
        w.dCode[i] = d["weather_code"][i] | 0;
        w.dMax[i] = d["temperature_2m_max"][i] | NAN;
        w.dMin[i] = d["temperature_2m_min"][i] | NAN;
        w.dRegen[i] = d["precipitation_sum"][i] | 0.0f;
        w.dRisiko[i] = d["precipitation_probability_max"][i] | 0;
        w.dUv[i] = d["uv_index_max"][i] | 0.0f;
        uhrzeit(d["sunrise"][i], w.dAufgang[i]);
        uhrzeit(d["sunset"][i], w.dUntergang[i]);
        w.nd++;
    }

    if (isnan(w.temp) || w.nh < 2 || w.nd < 2) {
        fehler = "Daten unvollstaendig";
        return false;
    }
    return true;
}
