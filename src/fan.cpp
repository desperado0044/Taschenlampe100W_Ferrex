#include <Arduino.h>
#include <math.h>

#include "fan.h"
#include "config.h"
#include "settings.h"

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
    } else {
        float onTemp = settingsGetFanOnTempC();
        float fullTemp = settingsGetFanFullTempC();
        if (heatsinkTempC <= onTemp) {
            percent = 0;
        } else if (heatsinkTempC >= fullTemp) {
            percent = 100;
        } else {
            float fraction = (heatsinkTempC - onTemp) / (fullTemp - onTemp);
            percent = (uint8_t)roundf(fraction * 100.0f);
        }
    }

    analogWrite(PIN_FAN_PWM, (uint16_t)percent * 255 / 100);
}

uint8_t fanGetPercent() {
    return percent;
}
