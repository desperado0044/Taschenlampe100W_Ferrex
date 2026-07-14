#include <Arduino.h>
#include <TFT_eSPI.h>

#include "config.h"

TFT_eSPI tft = TFT_eSPI();

#define PIN_LED_ONBOARD PC13

void setup() {
    Serial.begin(115200);

    pinMode(PIN_LED_ONBOARD, OUTPUT);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    // Deckender Textfarben-Hintergrund: jedes Zeichen überschreibt den alten Inhalt beim
    // Zeichnen direkt, kein separates fillScreen()+Neuzeichnen nötig -> kein Flackern.
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    analogReadResolution(12);
    pinMode(PIN_BATTERY_ADC, INPUT_ANALOG);

    pinMode(PIN_FAN_PWM, OUTPUT);
    pinMode(PIN_LED_PWM, OUTPUT);
    analogWrite(PIN_LED_PWM, 0);
}

void loop() {
    digitalWrite(PIN_LED_ONBOARD, HIGH);
    delay(300);
    digitalWrite(PIN_LED_ONBOARD, LOW);
    delay(300);

    // Mehrere Samples mitteln statt Einzelmessung, um ADC-Rauschen der hochohmigen
    // Spannungsteiler-Quelle wegzumitteln.
    uint32_t adcSum = 0;
    const uint8_t adcSamples = 16;
    for (uint8_t i = 0; i < adcSamples; i++) {
        adcSum += analogRead(PIN_BATTERY_ADC);
    }
    float adcRaw = (float)adcSum / adcSamples;
    float pinVoltage = adcRaw * 3.3f / 4095.0f;
    float batteryVoltage = pinVoltage * BATTERY_VOLTAGE_DIVIDER_RATIO;

    // Zusätzlich über die Update-Zyklen hinweg glätten (EMA) - der Akku (100Wh) ändert seine
    // Spannung ohnehin nur langsam, schnelle Reaktion ist hier nicht nötig.
    static float batteryVoltageSmoothed = NAN;
    const float emaAlpha = 0.1f;
    if (isnan(batteryVoltageSmoothed)) {
        batteryVoltageSmoothed = batteryVoltage;
    } else {
        batteryVoltageSmoothed += emaAlpha * (batteryVoltage - batteryVoltageSmoothed);
    }

    // Notabschaltung bei Unterspannung - einzige Schutzschicht, da der Akku selbst keinen
    // Zell-Cutoff hat. Hysterese verhindert Flattern nahe der Schwelle.
    static bool batteryUndervoltageLockout = false;
    if (!batteryUndervoltageLockout && batteryVoltageSmoothed < BATTERY_CUTOFF_VOLTAGE) {
        batteryUndervoltageLockout = true;
    } else if (batteryUndervoltageLockout && batteryVoltageSmoothed > BATTERY_CUTOFF_RECOVER_VOLTAGE) {
        batteryUndervoltageLockout = false;
    }

    // Vorwarnstufe: LED auf reduzierte Helligkeit dimmen statt hart abschalten.
    static bool batteryLowWarning = false;
    if (!batteryLowWarning && batteryVoltageSmoothed < BATTERY_LOW_WARNING_VOLTAGE) {
        batteryLowWarning = true;
    } else if (batteryLowWarning && batteryVoltageSmoothed > BATTERY_LOW_WARNING_RECOVER_VOLTAGE) {
        batteryLowWarning = false;
    }

    // TODO: gewünschte Helligkeit wird später vom Inkrementalgeber kommen - bis dahin fest 0.
    uint8_t desiredLedPercent = 0;
    uint8_t ledPercent = desiredLedPercent;
    if (batteryUndervoltageLockout) {
        ledPercent = 0;
    } else if (batteryLowWarning) {
        ledPercent = min(ledPercent, (uint8_t)BATTERY_LOW_WARNING_LED_PERCENT);
    }
    analogWrite(PIN_LED_PWM, (uint16_t)ledPercent * 255 / 100);

    // Feste Breite (Leerzeichen-gepolstert), damit bei wechselnder Ziffernanzahl (z.B.
    // 9.87 -> 10.12) keine Reste des alten Texts stehen bleiben.
    char line[16];
    snprintf(line, sizeof(line), "Akku: %5.2fV", batteryVoltageSmoothed);
    tft.setCursor(0, 0);
    tft.print(line);

    tft.setCursor(0, 16);
    if (batteryUndervoltageLockout) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("AKKU LEER!    ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    } else if (batteryLowWarning) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("AKKU SCHWACH  ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    } else {
        tft.print("              ");
    }
}
