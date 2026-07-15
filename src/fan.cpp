#include <Arduino.h>
#include <math.h>

#include "fan.h"
#include "config.h"

namespace {

uint8_t percent = 0;

}  // namespace

void fanInit() {
    pinMode(PIN_FAN_PWM, OUTPUT);
    analogWrite(PIN_FAN_PWM, 0);
}

void fanUpdate(float heatsinkTempC, bool ledOn) {
    if (!ledOn) {
        // Keine LED-Last - nichts zu kühlen, auch nicht bei hoher Umgebungstemperatur.
        percent = 0;
    } else if (isnan(heatsinkTempC)) {
        // Sensor liefert keinen gültigen Wert - im Zweifel lieber kühlen als riskieren, dass
        // die LED unbemerkt überhitzt.
        percent = 100;
    } else if (heatsinkTempC <= FAN_CURVE_MIN_TEMP_C) {
        percent = 0;
    } else if (heatsinkTempC >= FAN_CURVE_MAX_TEMP_C) {
        percent = 100;
    } else {
        float fraction = (heatsinkTempC - FAN_CURVE_MIN_TEMP_C) /
                          (FAN_CURVE_MAX_TEMP_C - FAN_CURVE_MIN_TEMP_C);
        percent = (uint8_t)roundf(fraction * 100.0f);
    }

    analogWrite(PIN_FAN_PWM, (uint16_t)percent * 255 / 100);
}

uint8_t fanGetPercent() {
    return percent;
}

void fanStop() {
    percent = 0;
    analogWrite(PIN_FAN_PWM, 0);
}
