#pragma once

void ledInit();

// LED ist immer aktiv (kein separater An/Aus-Zustand - dafür ist der Netzschalter da, siehe
// README). Ein Tasterdruck schaltet zwischen normaler Helligkeit und Turbo (100%) um. Drehen
// stellt die normale Helligkeit direkt ein (unabhängig vom Turbo-Zustand) - runtergedreht auf
// 0% dient als manuelles "Aus". batteryCeilingPercent/overtempCeilingPercent (siehe
// batteryGetLedCeilingPercent()/temperatureGetLedCeilingPercent()) deckeln die tatsächliche
// PWM-Leistung nach oben - es geht dabei um Leistungsreduzierung, nicht um empfundene
// Helligkeit, daher wirkt der Deckel vor der CIE-Korrektur (siehe led.cpp). Einmal pro
// Loop-Durchlauf aufrufen.
void ledUpdate(int32_t encoderDelta, bool shortPress, uint8_t batteryCeilingPercent,
               uint8_t overtempCeilingPercent);

// Tatsächlich ausgegebene Helligkeit in % (0-100), nach Anwendung der Akku-Limits - für die
// Anzeige.
uint8_t ledGetPercent();

// Geschätzte Leistungsaufnahme in Watt (PWM-Duty * LED_RATED_WATTS) - Näherung ohne
// Strommessung, für die Anzeige.
float ledGetWatts();
