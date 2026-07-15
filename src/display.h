#pragma once

// Initialisiert Backlight und TFT-Controller.
void displayInit();

// Zeigt Akkuspannung + Ladezustands-Meter. Wert und Balken sind statusabhängig eingefärbt
// (grün=normal, rot=nahe BATTERY_CUTOFF_VOLTAGE). throttleActive: zusätzlicher Textmarker,
// sobald die LED-Helligkeit spannungsbedingt gedeckelt wird.
void displayShowBattery(float voltage, uint8_t chargePercent, bool throttleActive);

// Zeigt die Kühlkörpertemperatur, statusabhängig eingefärbt (grün=normal, gelb=Lüfter am
// Limit, rot=nahe LED_THROTTLE_ZERO_C). tempC = NAN, falls der Sensor (noch) keinen gültigen
// Wert liefert. throttleActive: zusätzlicher Textmarker, sobald die LED-Helligkeit
// temperaturbedingt gedeckelt wird.
void displayShowTemperature(float tempC, bool throttleActive);

// Zeigt LED-Helligkeit + Meter sowie die geschätzte Leistungsaufnahme in Watt (siehe
// ledGetWatts()).
void displayShowLed(uint8_t percent, float watts);

// Zeigt Lüfter-PWM-Duty-Cycle + Meter.
void displayShowFan(uint8_t percent);

// Löscht den Bildschirm - bei jedem Wechsel zwischen Hauptanzeige, Menüliste und
// Werte-Bearbeitung aufrufen (unterschiedliches Layout, sonst blieben alte Pixel stehen).
void displayClear();

// Zeigt ein Fenster von Konfigurationspunkten (siehe MENU_VISIBLE_ROWS), der aktuell
// ausgewählte ist markiert (">", gelb) - names/count/selectedIndex beziehen sich auf das
// sichtbare Fenster, nicht auf die Gesamtliste. hasMoreAbove/hasMoreBelow blenden einen
// kleinen Scroll-Pfeil ein, falls die Liste über das Fenster hinausgeht.
const uint8_t MENU_VISIBLE_ROWS = 8;
void displayShowMenuList(const char* const* names, uint8_t count, uint8_t selectedIndex,
                          bool hasMoreAbove, bool hasMoreBelow);

// Zeigt den gerade bearbeiteten Konfigurationspunkt: Name, Wert + Einheit. previewVoltage:
// zusätzliche Zeile mit einer Live-Vorschau (z.B. Akkuspannung beim ADC-Korrekturfaktor) -
// NAN, falls für diesen Punkt nicht zutreffend. warning: Wert + Zusatzzeile rot, mit Verweis
// auf die README - für Werte oberhalb einer konservativen, datenblattfreien Vorgabe.
void displayShowMenuEdit(const char* itemName, float value, const char* unit,
                          float previewVoltage, bool warning);
