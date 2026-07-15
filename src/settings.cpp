#include <Arduino.h>
#include <math.h>
#include <EEPROM.h>

#include "settings.h"
#include "config.h"

namespace {

// Jeder Wert 4 Byte (float), fortlaufend - siehe loadFloat()/EEPROM.put() unten.
const int ADDR_BRIGHTNESS = 0;
const int ADDR_FAN_ON_C = 4;
const int ADDR_FAN_FULL_C = 8;
const int ADDR_LED_THROTTLE_START_C = 12;
const int ADDR_LED_THROTTLE_ZERO_C = 16;
const int ADDR_BATTERY_RAMP_START_V = 20;
const int ADDR_BATTERY_CUTOFF_V = 24;
const int ADDR_ADC_RATIO = 28;
const int ADDR_LED_RATED_WATTS = 32;
const int ADDR_BATTERY_FULL_V = 36;

float brightness = LED_DEFAULT_PERCENT;
float fanOnC = FAN_CURVE_MIN_TEMP_C;
float fanFullC = FAN_CURVE_MAX_TEMP_C;
float ledThrottleStartC = LED_THROTTLE_START_C;
float ledThrottleZeroC = LED_THROTTLE_ZERO_C;
float batteryRampStartV = BATTERY_LED_RAMP_START_VOLTAGE;
float batteryCutoffV = BATTERY_CUTOFF_VOLTAGE;
float adcRatio = BATTERY_VOLTAGE_DIVIDER_RATIO;
float ledRatedWatts = LED_RATED_WATTS;
float batteryFullV = BATTERY_FULL_VOLTAGE;

// Lädt einen Float aus dem EEPROM, behält den übergebenen Compile-Time-Fallback, falls der
// gespeicherte Wert ungültig ist (unbeschriebenes Flash liest als 0xFFFFFFFF, das als
// Bitmuster interpretiert NaN ergibt) oder unplausibel außerhalb der erlaubten Spanne liegt.
float loadFloat(int addr, float fallback, float minValid, float maxValid) {
    float value;
    EEPROM.get(addr, value);
    if (isnan(value) || value < minValid || value > maxValid) {
        return fallback;
    }
    return value;
}

}  // namespace

void settingsInit() {
    brightness = loadFloat(ADDR_BRIGHTNESS, LED_DEFAULT_PERCENT, 0.0f, 100.0f);
    fanOnC = loadFloat(ADDR_FAN_ON_C, FAN_CURVE_MIN_TEMP_C, 0.0f, 100.0f);
    fanFullC = loadFloat(ADDR_FAN_FULL_C, FAN_CURVE_MAX_TEMP_C, 0.0f, 100.0f);
    ledThrottleStartC = loadFloat(ADDR_LED_THROTTLE_START_C, LED_THROTTLE_START_C, 0.0f, 100.0f);
    ledThrottleZeroC = loadFloat(ADDR_LED_THROTTLE_ZERO_C, LED_THROTTLE_ZERO_C, 50.0f, 100.0f);
    batteryRampStartV =
        loadFloat(ADDR_BATTERY_RAMP_START_V, BATTERY_LED_RAMP_START_VOLTAGE, 15.0f, 21.0f);
    batteryCutoffV = loadFloat(ADDR_BATTERY_CUTOFF_V, BATTERY_CUTOFF_VOLTAGE, 15.0f, 18.0f);
    adcRatio = loadFloat(ADDR_ADC_RATIO, BATTERY_VOLTAGE_DIVIDER_RATIO, 1.0f, 20.0f);
    ledRatedWatts = loadFloat(ADDR_LED_RATED_WATTS, LED_RATED_WATTS, 1.0f, 300.0f);
    batteryFullV = loadFloat(ADDR_BATTERY_FULL_V, BATTERY_FULL_VOLTAGE, 5.0f, 60.0f);
}

float settingsGetDefaultBrightness() {
    return brightness;
}
void settingsSetDefaultBrightness(float percent) {
    brightness = percent;
    EEPROM.put(ADDR_BRIGHTNESS, brightness);
}

float settingsGetFanOnTempC() {
    return fanOnC;
}
void settingsSetFanOnTempC(float tempC) {
    fanOnC = tempC;
    EEPROM.put(ADDR_FAN_ON_C, fanOnC);
}

float settingsGetFanFullTempC() {
    return fanFullC;
}
void settingsSetFanFullTempC(float tempC) {
    fanFullC = tempC;
    EEPROM.put(ADDR_FAN_FULL_C, fanFullC);
}

float settingsGetLedThrottleStartC() {
    return ledThrottleStartC;
}
void settingsSetLedThrottleStartC(float tempC) {
    ledThrottleStartC = tempC;
    EEPROM.put(ADDR_LED_THROTTLE_START_C, ledThrottleStartC);
}

float settingsGetLedThrottleZeroC() {
    return ledThrottleZeroC;
}
void settingsSetLedThrottleZeroC(float tempC) {
    ledThrottleZeroC = tempC;
    EEPROM.put(ADDR_LED_THROTTLE_ZERO_C, ledThrottleZeroC);
}

float settingsGetBatteryRampStartVoltage() {
    return batteryRampStartV;
}
void settingsSetBatteryRampStartVoltage(float voltage) {
    batteryRampStartV = voltage;
    EEPROM.put(ADDR_BATTERY_RAMP_START_V, batteryRampStartV);
}

float settingsGetBatteryCutoffVoltage() {
    return batteryCutoffV;
}
void settingsSetBatteryCutoffVoltage(float voltage) {
    batteryCutoffV = voltage;
    EEPROM.put(ADDR_BATTERY_CUTOFF_V, batteryCutoffV);
}

float settingsGetAdcDividerRatio() {
    return adcRatio;
}
void settingsSetAdcDividerRatio(float ratio) {
    adcRatio = ratio;
    EEPROM.put(ADDR_ADC_RATIO, adcRatio);
}

float settingsGetLedRatedWatts() {
    return ledRatedWatts;
}
void settingsSetLedRatedWatts(float watts) {
    ledRatedWatts = watts;
    EEPROM.put(ADDR_LED_RATED_WATTS, ledRatedWatts);
}

float settingsGetBatteryFullVoltage() {
    return batteryFullV;
}
void settingsSetBatteryFullVoltage(float voltage) {
    batteryFullV = voltage;
    EEPROM.put(ADDR_BATTERY_FULL_V, batteryFullV);
}
