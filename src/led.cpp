#include <Arduino.h>
#include <math.h>

#include "led.h"
#include "config.h"

namespace {

bool ledEnabled = false;
bool turboActive = false;  // true = 100% statt desiredPercent, siehe ledUpdate()
// In Zehntel-Prozent geführt (0-1000), damit 40 Stufen exakt 2,5% pro Klick ergeben - mit
// einem uint8_t-Prozentwert (Schrittweite 5) ließen sich 40 Stufen sonst nicht rundungsfrei
// abbilden (100/40 = 2,5, keine Ganzzahl).
int32_t desiredPercentTenths = (int32_t)LED_DEFAULT_PERCENT * 10;
uint8_t actualPercent = 0;  // tatsächlich ausgegebene Helligkeit in % (nach Akku-/Temp-Deckel)
uint8_t actualPwm = 0;  // tatsächlicher PWM-Duty-Cycle (0-255) - Basis für ledGetWatts()

// 40 Stufen über den vollen Bereich statt 100 Einzelschritte - pro Klick 2,5%.
const int32_t PERCENT_STEP_TENTHS = 25;
const uint8_t TURBO_PERCENT = 100;

// Wandelt einen linear vom Nutzer eingestellten Prozentwert (0-100) in einen PWM-Duty-Cycle
// (0-255) um. Menschliche Helligkeitswahrnehmung ist nicht linear zur tatsächlichen
// Lichtleistung - ohne Korrektur würde der untere Bereich viel zu schnell heller wirken, der
// obere Bereich kaum merklich. CIE-1931-Lightness-Formel (Standardverfahren für
// wahrnehmungsgleichmäßige PWM-Dimmung).
uint8_t percentToPwm(uint8_t percent) {
    if (percent == 0) {
        return 0;
    }

    float lightness = percent;
    float linear;
    if (lightness <= 8.0f) {
        linear = lightness / 903.3f;
    } else {
        linear = powf((lightness + 16.0f) / 116.0f, 3.0f);
    }

    return (uint8_t)roundf(linear * 255.0f);
}

// Umkehrung von percentToPwm() - die Akku-/Temperatur-Obergrenzen in ledUpdate() deckeln die
// tatsächliche PWM-Leistung (linear zum LED-Strom), weil es dort um Leistungsreduzierung geht,
// nicht um empfundene Helligkeit. Für die Prozentanzeige wird das Ergebnis zurück in die
// wahrnehmungskorrigierte Skala gewandelt, damit der angezeigte Wert weiterhin stimmt.
uint8_t pwmToPercent(uint8_t pwm) {
    if (pwm == 0) {
        return 0;
    }

    float linear = pwm / 255.0f;
    float lightness;
    if (linear <= 0.008856f) {
        lightness = linear * 903.3f;
    } else {
        lightness = 116.0f * cbrtf(linear) - 16.0f;
    }

    return (uint8_t)roundf(lightness);
}

}  // namespace

void ledInit() {
    pinMode(PIN_LED_PWM, OUTPUT);
    analogWrite(PIN_LED_PWM, 0);
}

void ledUpdate(int32_t encoderDelta, bool shortPress, bool longPress,
               uint8_t batteryCeilingPercent, uint8_t overtempCeilingPercent) {
    if (shortPress) {
        if (ledEnabled) {
            // Bereits an: kurzer Druck schaltet zwischen normaler Helligkeit und Turbo (100%)
            // um, statt die LED auszuschalten.
            turboActive = !turboActive;
        } else {
            ledEnabled = true;
            turboActive = false;
        }
    }
    if (longPress) {
        ledEnabled = false;
        turboActive = false;
    }

    if (encoderDelta != 0) {
        int32_t newTenths = desiredPercentTenths + encoderDelta * PERCENT_STEP_TENTHS;
        desiredPercentTenths = constrain(newTenths, (int32_t)0, (int32_t)1000);
    }

    uint8_t desiredPercent = (uint8_t)((desiredPercentTenths + 5) / 10);  // auf ganze % runden
    uint8_t targetPercent = turboActive ? TURBO_PERCENT : desiredPercent;
    uint8_t requestedPwm = ledEnabled ? percentToPwm(targetPercent) : 0;

    // Akku-/Temperatur-Obergrenzen auf die tatsächliche PWM-Leistung anwenden (linear zum
    // LED-Strom), nicht auf den wahrnehmungskorrigierten Prozentwert - hier geht es um
    // Leistungsreduzierung, nicht um empfundene Helligkeit. Deckeln nur nach oben, heben eine
    // bereits niedrigere Anforderung nie an.
    uint8_t batteryCeilingPwm = (uint8_t)((uint16_t)batteryCeilingPercent * 255 / 100);
    uint8_t overtempCeilingPwm = (uint8_t)((uint16_t)overtempCeilingPercent * 255 / 100);
    uint8_t pwm = min(requestedPwm, min(batteryCeilingPwm, overtempCeilingPwm));

    actualPercent = pwmToPercent(pwm);
    actualPwm = pwm;
    analogWrite(PIN_LED_PWM, pwm);
}

uint8_t ledGetPercent() {
    return actualPercent;
}

float ledGetWatts() {
    return (float)actualPwm / 255.0f * LED_RATED_WATTS;
}

bool ledIsEnabled() {
    return ledEnabled;
}
