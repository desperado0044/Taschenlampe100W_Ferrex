#pragma once

// Alle Pins sind vorläufig - Board ist bereits verdrahtet, aber die Belegung muss noch am
// Bluepill nachgemessen werden. Werte unten sind ein Vorschlag, der Timer-/SPI-Vorgaben
// respektiert und PA9/PA10 (Serial-Debug via USART1) sowie PA13/PA14 (SWD zum ST-Link) frei
// lässt. Nach dem Nachmessen hier einfach die echten Pins eintragen.

// 1,8" TFT 128x160, SPI - läuft über SPI1 (feste Pins SCK/MOSI, CS/DC/RST frei wählbar).
// Treiber-IC angenommen: ST7735 (Standardchip bei diesen Modulen). Falls es ein ST7789 o.ä.
// ist, Library in platformio.ini entsprechend anpassen.
// Nachgemessen: CS=A4, RESET=B10, A0(=DC)=B11, SDA(=MOSI)=A7, SCK=A5, LED(=Backlight)=B1.
#define TFT_SCLK PA5
#define TFT_MOSI PA7
#define TFT_CS   PA4
#define TFT_DC   PB11
#define TFT_RST  PB10
#define TFT_BL   PB1  // Backlight, liegt auf TIM3_CH4 - später bei Bedarf per PWM dimmbar.

// MOSFET Lüftersteuerung - nachgemessen: A8 (TIM1_CH1).
#define PIN_FAN_PWM PA8

// MOSFET 100W-LED-Steuerung - nachgemessen: A11 (TIM1_CH4). Liegt auf demselben Timer wie
// der Lüfter (TIM1) - PWM-Frequenz ist dadurch an die des Lüfters gekoppelt, Duty Cycle
// bleibt pro Kanal unabhängig einstellbar.
#define PIN_LED_PWM PA11

// Akkuspannung über Spannungsteiler - nachgemessen: B0 (ADC1_IN8). Teiler ist einstellbar
// (Trimmer), auf ~3,0V Pin-Spannung bei max. Akkuspannung (~22V) ausgelegt für volle
// ADC-Auflösung. Fein-kalibriert gegen Multimeter (nach vollem EMA-Einschwingen, ~30-40s):
// Anzeige 20,00V vs. gemessen 20,179V -> Korrektur 20,179/20,00 = 1,00895
// -> Faktor 7,83 * 1,00895 = 7,90.
// Bei erneutem Verstellen des Trimmers muss dieser Faktor neu kalibriert werden.
#define PIN_BATTERY_ADC PB0
#define BATTERY_VOLTAGE_DIVIDER_RATIO 7.90f

// Notabschaltung bei Unterspannung: Der Ferrex-Akku hat laut Recherche keinen internen
// BMS-Zellschutz (nur NTC, kein Balancer/Cutoff) - diese Schwelle ist daher die einzige
// Schutzschicht gegen Tiefentladung, nicht nur ein Backup. 15,0V = 3,0V/Zelle bei 5S.
// Hysterese, damit die Abschaltung nicht bei jeder kleinen Schwankung an/aus flattert.
#define BATTERY_CUTOFF_VOLTAGE 15.0f
#define BATTERY_CUTOFF_RECOVER_VOLTAGE 15.5f

// Vorwarnstufe oberhalb des harten Cutoffs: LED wird auf reduzierte Helligkeit gedimmt,
// statt abrupt abzuschalten - Vorwarnung für den Nutzer, bevor der Cutoff greift.
#define BATTERY_LOW_WARNING_VOLTAGE 16.0f
#define BATTERY_LOW_WARNING_RECOVER_VOLTAGE 16.5f
#define BATTERY_LOW_WARNING_LED_PERCENT 10

// TODO: DS18B20 (Kühlkörper) und Inkrementalgeber (A/B/Taster) noch nachmessen.
