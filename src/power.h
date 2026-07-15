#pragma once

// Schaltet das Backlight aus und versetzt die MCU in den STM32-STOP-Modus (SRAM/Register
// bleiben erhalten, alle Takte im 1,8V-Domain angehalten). Kehrt erst zurück, sobald ein
// EXTI-Interrupt (der bereits eingerichtete Taster-Interrupt) sie aufweckt - stellt danach
// den 72MHz-Systemtakt wieder her (fällt im STOP-Modus auf den internen 8MHz-Oszillator
// zurück) und schaltet das Backlight wieder ein.
void powerEnterDeepSleep();
