#include <Arduino.h>

#include "encoder.h"
#include "config.h"

namespace {

// TIM2 zählt die Quadratur-Pulse komplett in Hardware (kein Interrupt/CPU-Overhead) - CLK=A0
// und DT=A1 sind auf diesem Chip zufällig genau TIM2_CH1/CH2, kein Pin-Remap nötig. Löst das
// Problem der Software-/Interrupt-Lösung, dass bei schnellem Drehen Übergänge verloren gehen
// konnten.
TIM_HandleTypeDef timHandle;
int16_t lastCounter = 0;
int32_t stepRemainder = 0;

// Der Haupt-Loop ist durch die Display-Zeichnerei mittlerweile zu langsam (100-300ms), um den
// Taster zuverlässig per reinem Polling abzutasten - kurze Drücke gingen dazwischen verloren.
// Ein Interrupt fängt jede Flanke garantiert ein, unabhängig von der Loop-Geschwindigkeit; die
// Entprellung selbst läuft weiterhin zeitbasiert in encoderIsButtonPressed().
volatile int isrRawState = HIGH;
volatile unsigned long isrChangeMs = 0;

void onButtonChange() {
    isrRawState = digitalRead(PIN_ENC_SW);
    isrChangeMs = millis();
}

}  // namespace

void encoderInit() {
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    __HAL_RCC_TIM2_CLK_ENABLE();

    timHandle.Instance = TIM2;
    timHandle.Init.Prescaler = 0;
    timHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    timHandle.Init.Period = 0xFFFF;
    timHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    TIM_Encoder_InitTypeDef encoderConfig = {0};
    encoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;  // beide Kanäle, beide Flanken
    encoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    encoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    encoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    encoderConfig.IC1Filter = 10;  // Digitalfilter gegen Kontaktprellen
    encoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    encoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    encoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    encoderConfig.IC2Filter = 10;

    HAL_TIM_Encoder_Init(&timHandle, &encoderConfig);
    HAL_TIM_Encoder_Start(&timHandle, TIM_CHANNEL_ALL);

    lastCounter = (int16_t)__HAL_TIM_GET_COUNTER(&timHandle);

    isrRawState = digitalRead(PIN_ENC_SW);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_SW), onButtonChange, CHANGE);
}

int32_t encoderReadDelta() {
    int16_t currentCounter = (int16_t)__HAL_TIM_GET_COUNTER(&timHandle);
    // Differenz als int16_t bilden, damit ein Überlauf des 16-Bit-Zählers korrekt als
    // vorzeichenbehaftete Differenz interpretiert wird.
    int16_t rawDelta = currentCounter - lastCounter;
    lastCounter = currentCounter;

    // Wie bei der vorherigen Software-Lösung ermittelt: 2 Zähler-Schritte pro fühlbarem
    // Rastpunkt bei diesem Encoder. Rest bleibt für den nächsten Aufruf stehen.
    stepRemainder += rawDelta;
    int32_t steps = stepRemainder / 2;
    stepRemainder -= steps * 2;

    return steps;
}

namespace {

bool debouncedPressed = false;
unsigned long pressStartMs = 0;
bool longPressFired = false;
bool shortPressPending = false;
bool longPressPending = false;

}  // namespace

bool encoderIsButtonPressed() {
    noInterrupts();
    int rawState = isrRawState;
    unsigned long changeMs = isrChangeMs;
    interrupts();

    if (millis() - changeMs > ENCODER_BUTTON_DEBOUNCE_MS) {
        bool newDebounced = (rawState == LOW);
        if (newDebounced && !debouncedPressed) {
            // Steigende Flanke (entprellt) - Druck beginnt.
            pressStartMs = millis();
            longPressFired = false;
        } else if (!newDebounced && debouncedPressed) {
            // Fallende Flanke - Druck endet. Nur als kurzer Druck werten, wenn währenddessen
            // nicht schon ein langer Druck ausgelöst wurde.
            if (!longPressFired) {
                shortPressPending = true;
            }
        }
        debouncedPressed = newDebounced;
    }

    // Langen Druck sofort erkennen, während der Taster noch gehalten wird - nicht erst beim
    // Loslassen.
    if (debouncedPressed && !longPressFired &&
        (millis() - pressStartMs) >= ENCODER_LONG_PRESS_MS) {
        longPressFired = true;
        longPressPending = true;
    }

    return debouncedPressed;
}

bool encoderConsumeShortPress() {
    if (shortPressPending) {
        shortPressPending = false;
        return true;
    }
    return false;
}

bool encoderConsumeLongPress() {
    if (longPressPending) {
        longPressPending = false;
        return true;
    }
    return false;
}
