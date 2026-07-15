#include <Arduino.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "temperature.h"
#include "config.h"

namespace {

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);

float heatsinkTempC = NAN;
bool conversionInProgress = false;
unsigned long conversionStartMs = 0;

// DS18B20-Wandlungszeit bei 12-Bit-Auflösung (Werkseinstellung). Non-blocking abgewartet,
// damit der schnelle Hauptloop (~100ms) nicht auf die Konvertierung wartet.
const unsigned long CONVERSION_TIME_MS = 750;

}  // namespace

void temperatureInit() {
    sensors.begin();
    sensors.setWaitForConversion(false);
}

void temperatureUpdate() {
    unsigned long now = millis();

    if (!conversionInProgress) {
        sensors.requestTemperatures();
        conversionStartMs = now;
        conversionInProgress = true;
        return;
    }

    if (now - conversionStartMs < CONVERSION_TIME_MS) {
        return;
    }

    float reading = sensors.getTempCByIndex(0);
    heatsinkTempC = (reading == DEVICE_DISCONNECTED_C) ? NAN : reading;
    conversionInProgress = false;
}

float temperatureGetHeatsinkC() {
    return heatsinkTempC;
}

uint8_t temperatureGetLedCeilingPercent() {
    // Sensor ungültig (getrennt/kein gültiger Messwert) - sicherheitshalber wie am heißen Ende
    // der Rampe behandeln, analog zur Lüfterkurve bei NAN.
    if (isnan(heatsinkTempC)) {
        return 0;
    }
    if (heatsinkTempC <= LED_THROTTLE_START_C) {
        return 100;
    }
    if (heatsinkTempC >= LED_THROTTLE_ZERO_C) {
        return 0;
    }

    float fraction = (heatsinkTempC - LED_THROTTLE_START_C) /
                      (LED_THROTTLE_ZERO_C - LED_THROTTLE_START_C);
    return (uint8_t)roundf((1.0f - fraction) * 100.0f);
}
