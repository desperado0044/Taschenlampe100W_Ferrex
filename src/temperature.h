#pragma once

// Initialisiert den OneWire-Bus und sucht den DS18B20 auf dem Kühlkörper.
void temperatureInit();

// Stößt eine neue Messung an und liest sie aus. Einmal pro Loop-Durchlauf aufrufen.
void temperatureUpdate();

// Zuletzt gemessene Kühlkörpertemperatur in °C, oder NAN falls (noch) kein gültiger
// Messwert vorliegt (Sensor nicht gefunden/getrennt).
float temperatureGetHeatsinkC();

// Temperaturbedingte Obergrenze für die LED-Helligkeit in % (0-100): 100 = keine Einschränkung
// (Temperatur <= LED_THROTTLE_START_C), linear fallend bis 0 bei LED_THROTTLE_ZERO_C oder bei
// ungültiger Messung. Wirkt nur als Deckel, hebt eine niedrigere eingestellte Helligkeit nie an.
uint8_t temperatureGetLedCeilingPercent();
