#include <Arduino.h>

#include "config.h"
#include "battery.h"
#include "temperature.h"
#include "encoder.h"
#include "led.h"
#include "fan.h"
#include "display.h"
#include "power.h"

#define PIN_LED_ONBOARD PC13

namespace {

// Gemeinsame Aufwach-Sequenz nach powerEnterDeepSleep() - sowohl beim regulären Einschlafen
// (langer Druck im laufenden Betrieb) als auch direkt nach dem Booten (siehe setup()): Encoder-
// /Taster-Basis neu referenzieren (siehe encoderResync()), dann den weckenden Druck direkt als
// "kurzer Druck" werten (LED an) - der wirkt durch die Neu-Referenzierung sonst verloren,
// obwohl er ja gerade erst geweckt hat.
void sleepThenWakeOn() {
    // Lüfter explizit auf 0 vor dem Schlafen - sonst friert der Timer im STOP-Modus mitten im
    // PWM-Zyklus ein und kann auf "high" hängen bleiben (siehe fan.h). Die LED ist an dieser
    // Stelle bereits auf 0 (siehe Aufrufer), Kühlung wird beim Schlafen ohnehin nicht gebraucht.
    fanStop();

    powerEnterDeepSleep();
    encoderResync();
    ledUpdate(0, true, false, batteryGetLedCeilingPercent(), temperatureGetLedCeilingPercent());
}

}  // namespace

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
    ledInit();
    fanInit();

    // Direkt nach dem Booten in den Schlaf, statt die LED sofort mit LED_DEFAULT_PERCENT
    // einzuschalten: Der Spannungswandler zwischen Akku und Board hält die Spannung bei
    // Stromverlust lange und bricht dann abrupt weg - zu wenig Vorwarnzeit für einen
    // zuverlässigen EEPROM-Notspeichervorgang, daher bewusst kein Versuch, die Helligkeit über
    // Stromverlust hinweg zu retten (siehe LED_DEFAULT_PERCENT in config.h). Der erste
    // Tasterdruck weckt auf und schaltet die LED mit dem Startwert ein.
    sleepThenWakeOn();
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

    // Nur einmal pro Loop konsumieren - der Rückgabewert wird sowohl an ledUpdate() als auch
    // an die Debug-Anzeige weitergereicht, ein zweiter Consume-Aufruf würde das Ereignis vor
    // dem jeweils anderen Verbraucher verschlucken.
    encoderIsButtonPressed();
    bool shortPress = encoderConsumeShortPress();
    bool longPress = encoderConsumeLongPress();

    ledUpdate(encoderReadDelta(), shortPress, longPress, batteryGetLedCeilingPercent(),
              temperatureGetLedCeilingPercent());

    // Nach ledUpdate() aufrufen, damit ledIsEnabled() den aktuellen (nicht den vom letzten
    // Durchlauf) Zustand widerspiegelt.
    fanUpdate(temperatureGetHeatsinkC(), ledIsEnabled());

    if (longPress) {
        // ledUpdate() hat die LED oben schon auf 0 gesetzt - Anzeige einmal aktualisieren,
        // bevor der Takt für den Schlaf heruntergefahren wird, dann schlafen.
        displayShowLed(ledGetPercent(), ledGetWatts());

        // Warten, bis der Taster wirklich losgelassen (entprellt) ist - sonst könnte eine
        // Prell-Flanke beim Loslassen die MCU sofort wieder aufwecken.
        while (encoderIsButtonPressed()) {
        }

        sleepThenWakeOn();
    }

    displayShowBattery(batteryGetVoltageSmoothed(), batteryGetChargePercent(),
                        batteryGetLedCeilingPercent() < 100);
    displayShowTemperature(temperatureGetHeatsinkC(), temperatureGetLedCeilingPercent() < 100);
    displayShowLed(ledGetPercent(), ledGetWatts());
    displayShowFan(fanGetPercent());
}
