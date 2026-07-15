#include <Arduino.h>

#include "config.h"
#include "battery.h"
#include "temperature.h"
#include "encoder.h"
#include "led.h"
#include "fan.h"
#include "display.h"
#include "settings.h"
#include "menu.h"

#define PIN_LED_ONBOARD PC13

void setup() {
    pinMode(PIN_LED_ONBOARD, OUTPUT);

    // Über den Hörbereich hinaus (Default 1000Hz war als hörbares Fiepen wahrnehmbar). Muss
    // vor dem ersten analogWrite() auf PIN_FAN_PWM/PIN_LED_PWM gesetzt werden - beide teilen
    // sich TIM1, gilt also automatisch für beide. Der LED-MOSFET sitzt mit auf dem Kühlkörper
    // (verkraftet die höheren Schaltverluste bei 50kHz).
    analogWriteFrequency(50000);

    displayInit();
    batteryInit();
    temperatureInit();
    encoderInit();
    settingsInit();  // vor ledInit() - ledInit() liest den Startwert von dort.
    ledInit();
    fanInit();
    menuInit();
}

void loop() {
    // Nicht-blockierendes Blinken statt delay() - jede Blockierung im Loop verzögert direkt
    // die Taster-/Encoder-Reaktion, da deren Abfrage denselben Durchlauf teilt.
    static unsigned long lastBlinkMs = 0;
    static bool blinkState = false;
    unsigned long now = millis();
    if (now - lastBlinkMs >= 300) {
        blinkState = !blinkState;
        digitalWrite(PIN_LED_ONBOARD, blinkState);
        lastBlinkMs = now;
    }

    batteryUpdate();
    temperatureUpdate();

    encoderIsButtonPressed();
    bool shortPress = encoderConsumeShortPress();
    bool longPress = encoderConsumeLongPress();
    int32_t encoderDelta = encoderReadDelta();

    static UiMode previousMode = UiMode::Main;
    UiMode mode = menuUpdate(encoderDelta, shortPress, longPress);
    if (mode != previousMode) {
        // Jeder Moduswechsel (Haupt/Menüliste/Bearbeiten) hat ein anderes Layout, alte Pixel
        // blieben sonst stehen.
        displayClear();
    }
    previousMode = mode;

    // Encoder/Taster nur im Main-Modus an die LED weiterreichen - im Menü steuern sie die
    // Punkteauswahl bzw. den bearbeiteten Wert (siehe menuUpdate()), die Helligkeit bleibt
    // währenddessen unverändert stehen.
    bool ledActive = (mode == UiMode::Main);
    ledUpdate(ledActive ? encoderDelta : 0, ledActive && shortPress,
              batteryGetLedCeilingPercent(), temperatureGetLedCeilingPercent());

    // Nach ledUpdate() aufrufen, damit ledGetPercent() den aktuellen (nicht den vom letzten
    // Durchlauf) Zustand widerspiegelt.
    fanUpdate(temperatureGetHeatsinkC(), ledGetPercent() > 0);

    if (mode == UiMode::Main) {
        displayShowBattery(batteryGetVoltageSmoothed(), batteryGetChargePercent(),
                            batteryGetLedCeilingPercent() < 100);
        displayShowTemperature(temperatureGetHeatsinkC(), temperatureGetLedCeilingPercent() < 100);
        displayShowLed(ledGetPercent(), ledGetWatts());
        displayShowFan(fanGetPercent());
    } else {
        menuShow();
    }
}
