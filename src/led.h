#pragma once

void ledInit();

// Kurzer Druck: an, falls aus - falls bereits an, Umschalten zwischen normaler Helligkeit und
// Turbo (100%). Langer Druck: aus. Drehen stellt die normale Helligkeit direkt ein (unabhängig
// vom An/Aus-/Turbo-Zustand). batteryCeilingPercent/overtempCeilingPercent (siehe
// batteryGetLedCeilingPercent()/temperatureGetLedCeilingPercent()) deckeln die tatsächliche
// PWM-Leistung nach oben - es geht dabei um Leistungsreduzierung, nicht um empfundene
// Helligkeit, daher wirkt der Deckel vor der CIE-Korrektur (siehe led.cpp). Einmal pro
// Loop-Durchlauf aufrufen.
void ledUpdate(int32_t encoderDelta, bool shortPress, bool longPress,
               uint8_t batteryCeilingPercent, uint8_t overtempCeilingPercent);

// Tatsächlich ausgegebene Helligkeit in % (0-100), nach Anwendung der Akku-Limits - für die
// Anzeige.
uint8_t ledGetPercent();

// True, wenn der Nutzer die LED eingeschaltet hat (unabhängig von einer eventuellen
// Akku-/Temperatur-Drosselung auf 0%) - für fanUpdate(): der Lüfter soll nur laufen, wenn die
// LED an ist, nicht schon bei hoher Umgebungstemperatur ohne LED-Last.
bool ledIsEnabled();

// Geschätzte Leistungsaufnahme in Watt (PWM-Duty * LED_RATED_WATTS) - Näherung ohne
// Strommessung, für die Anzeige.
float ledGetWatts();
