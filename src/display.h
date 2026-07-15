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
