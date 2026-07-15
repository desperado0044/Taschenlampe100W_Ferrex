#include <Arduino.h>
#include <math.h>

#include "battery.h"
#include "config.h"
#include "settings.h"

namespace {

// Ringpuffer für den gleitenden Mittelwert über die Update-Zyklen hinweg - im Gegensatz zu
// einer EMA nach genau HISTORY_SIZE Zyklen vollständig eingeschwungen, kein asymptotischer
// Schwanz.
const uint8_t HISTORY_SIZE = 10;
float voltageHistory[HISTORY_SIZE];
uint8_t historyIndex = 0;
bool historyFilled = false;

float voltageSmoothed = NAN;

}  // namespace

void batteryInit() {
    analogReadResolution(12);
    pinMode(PIN_BATTERY_ADC, INPUT_ANALOG);

    // Hochohmig (Teiler offen) als Ruhezustand - keine Parasitärlast am Akku außerhalb
    // der eigentlichen Messung.
    pinMode(PIN_BATTERY_GND, INPUT);
}

void batteryUpdate() {
    // Teiler-GND schließen, RC-Einschwingzeit abwarten, dann mitteln, danach sofort wieder
    // öffnen - hält die Zeit, in der der Teiler tatsächlich Strom zieht, minimal.
    pinMode(PIN_BATTERY_GND, OUTPUT);
    digitalWrite(PIN_BATTERY_GND, LOW);
    delayMicroseconds(BATTERY_GND_SETTLE_TIME_US);

    // Mehrere Samples mitteln statt Einzelmessung, um ADC-Rauschen der hochohmigen
    // Spannungsteiler-Quelle wegzumitteln.
    uint32_t adcSum = 0;
    const uint8_t adcSamples = 16;
    for (uint8_t i = 0; i < adcSamples; i++) {
        adcSum += analogRead(PIN_BATTERY_ADC);
    }
    pinMode(PIN_BATTERY_GND, INPUT);

    float adcRaw = (float)adcSum / adcSamples;
    float pinVoltage = adcRaw * 3.3f / 4095.0f;
    float voltage = pinVoltage * settingsGetAdcDividerRatio();

    // Zusätzlich über die Update-Zyklen hinweg glätten (gleitender Mittelwert) - der Akku
    // (100Wh) ändert seine Spannung ohnehin nur langsam, schnelle Reaktion ist hier nicht
    // nötig. Puffer beim allerersten Aufruf komplett mit dem ersten Messwert vorfüllen,
    // damit von Anfang an ein gültiger Mittelwert über HISTORY_SIZE Werte ansteht.
    if (!historyFilled) {
        for (uint8_t i = 0; i < HISTORY_SIZE; i++) {
            voltageHistory[i] = voltage;
        }
        historyFilled = true;
    } else {
        voltageHistory[historyIndex] = voltage;
        historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    }

    float historySum = 0.0f;
    for (uint8_t i = 0; i < HISTORY_SIZE; i++) {
        historySum += voltageHistory[i];
    }
    voltageSmoothed = historySum / HISTORY_SIZE;
}

float batteryGetVoltageSmoothed() {
    return voltageSmoothed;
}

uint8_t batteryGetLedCeilingPercent() {
    if (isnan(voltageSmoothed)) {
        return 0;
    }
    float rampStart = settingsGetBatteryRampStartVoltage();
    float cutoff = settingsGetBatteryCutoffVoltage();
    if (voltageSmoothed >= rampStart) {
        return 100;
    }
    if (voltageSmoothed <= cutoff) {
        return 0;
    }

    float fraction = (voltageSmoothed - cutoff) / (rampStart - cutoff);
    return (uint8_t)roundf(fraction * 100.0f);
}

uint8_t batteryGetChargePercent() {
    if (isnan(voltageSmoothed)) {
        return 0;
    }
    float percent = (voltageSmoothed - settingsGetBatteryCutoffVoltage()) /
                     (settingsGetBatteryFullVoltage() - settingsGetBatteryCutoffVoltage()) *
                     100.0f;
    percent = constrain(percent, 0.0f, 100.0f);
    return (uint8_t)roundf(percent);
}
