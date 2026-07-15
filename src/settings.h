#pragma once

// Liest alle Werte aus dem EEPROM (Fallback auf die Compile-Time-Vorgaben aus config.h, falls
// noch nichts gespeichert wurde oder der gespeicherte Wert unplausibel ist). Vor dem ersten
// Gebrauch der Getter unten aufrufen.
void settingsInit();

// Alle Werte werden als float geführt (auch Prozentwerte), damit sie sich einheitlich über
// eine generische Menü-Tabelle (siehe menu.cpp) bearbeiten lassen. Jeder Setter schreibt
// sofort ins EEPROM - das ist hier unproblematisch, weil er nur bei bewusster Bestätigung im
// Konfigurationsbildschirm aufgerufen wird (kein häufiger/automatischer Schreibpfad wie bei
// der früher verworfenen Live-Persistenz).

float settingsGetDefaultBrightness();
void settingsSetDefaultBrightness(float percent);

float settingsGetFanOnTempC();
void settingsSetFanOnTempC(float tempC);

float settingsGetFanFullTempC();
void settingsSetFanFullTempC(float tempC);

float settingsGetLedThrottleStartC();
void settingsSetLedThrottleStartC(float tempC);

float settingsGetLedThrottleZeroC();
void settingsSetLedThrottleZeroC(float tempC);

float settingsGetBatteryRampStartVoltage();
void settingsSetBatteryRampStartVoltage(float voltage);

float settingsGetBatteryCutoffVoltage();
void settingsSetBatteryCutoffVoltage(float voltage);

float settingsGetAdcDividerRatio();
void settingsSetAdcDividerRatio(float ratio);

float settingsGetLedRatedWatts();
void settingsSetLedRatedWatts(float watts);

float settingsGetBatteryFullVoltage();
void settingsSetBatteryFullVoltage(float voltage);
