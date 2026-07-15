#include <Arduino.h>

#include "power.h"
#include "config.h"

// In variant_PILL_F103Cx.cpp definiert (WEAK, nicht static) - konfiguriert HSE+PLL auf
// 72MHz, exakt das, was nach dem STOP-Modus wiederhergestellt werden muss.
extern "C" void SystemClock_Config(void);

void powerEnterDeepSleep() {
    digitalWrite(TFT_BL, LOW);

    // SysTick (für millis()) feuert alle 1ms und würde WFI sofort wieder aufwecken, ohne das
    // hier anzuhalten - der Schlaf würde nie "halten", sondern im 1ms-Takt flackern.
    HAL_SuspendTick();

    // STOP-Modus statt STANDBY: SRAM und alle Variablenzustände bleiben erhalten, wacht per
    // beliebigem EXTI-Interrupt auf (der Taster-Interrupt aus encoder.cpp ist bereits
    // eingerichtet, keine zusätzliche Wakeup-Konfiguration nötig).
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    // Ab hier läuft der Code nach dem Aufwachen weiter - der Takt ist auf den internen
    // 8MHz-Oszillator zurückgefallen und muss neu konfiguriert werden.
    SystemClock_Config();
    HAL_ResumeTick();

    digitalWrite(TFT_BL, HIGH);
}
