#pragma once

// Initialisiert den ADC-Pin für die Akkuspannungsmessung.
void batteryInit();

// Liest den ADC, mittelt und aktualisiert den internen Zustand (Glättung). Einmal pro
// Loop-Durchlauf aufrufen.
void batteryUpdate();

float batteryGetVoltageSmoothed();

// Spannungsbedingte Obergrenze für die LED-Helligkeit in % (0-100): 100 = keine Einschränkung
// (Spannung >= BATTERY_LED_RAMP_START_VOLTAGE), linear fallend bis 0 bei
// BATTERY_CUTOFF_VOLTAGE oder bei ungültiger Messung. Einzige Schutzschicht gegen
// Tiefentladung, da der Akku selbst keinen Zell-Cutoff hat - wirkt nur als Deckel, hebt eine
// niedrigere eingestellte Helligkeit nie an.
uint8_t batteryGetLedCeilingPercent();

// Ladezustand in % (0 = Cutoff-Schwelle, 100 = volle Ladeschlussspannung) - für die Anzeige.
uint8_t batteryGetChargePercent();
