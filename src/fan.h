#pragma once

void fanInit();

// Setzt die Lüfter-PWM anhand der Kühlkörpertemperatur (lineare Kurve zwischen
// FAN_CURVE_MIN_TEMP_C und FAN_CURVE_MAX_TEMP_C), aber nur solange ledOn true ist - ist die
// LED aus, gibt es keine LED-Last zu kühlen, der Lüfter soll dann auch bei hoher
// Umgebungstemperatur nicht laufen. Bei ungültiger Temperatur (NAN, z.B. Sensor getrennt)
// sicherheitshalber volle Drehzahl statt aus (nur solange ledOn true ist). Einmal pro
// Loop-Durchlauf aufrufen.
void fanUpdate(float heatsinkTempC, bool ledOn);

// Aktuell kommandierter PWM-Duty-Cycle in % - für die Anzeige.
uint8_t fanGetPercent();

// Erzwingt 0% Duty-Cycle - vor powerEnterDeepSleep() aufrufen: der Timer friert im STOP-Modus
// mitten im PWM-Zyklus ein und kann sonst auf "high" hängen bleiben, statt sauber auf 0 zu
// gehen (LED ist beim Schlafen ohnehin aus, Kühlung wird nicht gebraucht).
void fanStop();
