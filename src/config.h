// Einstellungen der Wetterstation
#pragma once

// Ort (ORT_NAME, ORT_LAT, ORT_LON) und WLAN stehen in include/secrets.h (nicht im Repo)
#include "secrets.h"

#if !defined(ORT_NAME) || !defined(ORT_LAT) || !defined(ORT_LON)
#error "ORT_NAME, ORT_LAT und ORT_LON in include/secrets.h eintragen (Vorlage: secrets.example.h)"
#endif

// Zeitzone Deutschland (MEZ/MESZ) und NTP-Server
#define ZEITZONE   "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SERVER "pool.ntp.org"

// Aktualisierung zur vollen und halben Stunde (+1 min), bei Fehlern nach 5 min erneut
#define INTERVALL_S     1800
#define RETRY_S         300

// Reinigung gegen Geisterbilder: eine Runde pro Nacht zu dieser Stunde
#define REINIGUNG_STUNDE 3

// Temperatur am Aufstellort des Panels (fuer die Ansteuerung, nicht die Aussentemperatur)
#define PANEL_TEMPERATUR 21.0f
