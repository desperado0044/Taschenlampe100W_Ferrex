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

// MOSFET Lüftersteuerung (schaltet Rot/Schwarz des 4-Draht-PC-Lüfters, PWM per MOSFET statt
// über den eigentlich dafür vorgesehenen Blau-Steuereingang - vorläufig so belassen, sauberer
// wäre eine Umverdrahtung auf den PWM-Steuereingang des Lüfters) - nachgemessen: A8 (TIM1_CH1).
#define PIN_FAN_PWM PA8

// MOSFET 100W-LED-Steuerung - nachgemessen: A11 (TIM1_CH4). Liegt auf demselben Timer wie
// der Lüfter (TIM1) - PWM-Frequenz ist dadurch an die des Lüfters gekoppelt, Duty Cycle
// bleibt pro Kanal unabhängig einstellbar.
#define PIN_LED_PWM PA11

// Nennleistung bei 100% PWM-Duty-Cycle - für die geschätzte Watt-Anzeige (Duty * Nennleistung).
// Kein echter Strommesswert (kein Shunt vorhanden), nur eine Näherung.
#define LED_RATED_WATTS 100.0f

// Lüfterkurve: unterhalb FAN_CURVE_MIN_TEMP_C aus, linear hochlaufend bis 100% bei
// FAN_CURVE_MAX_TEMP_C.
#define FAN_CURVE_MIN_TEMP_C 40.0f
#define FAN_CURVE_MAX_TEMP_C 65.0f

// Lüfter-Tacho (gelb) liegt an B9, wird aber softwareseitig nicht ausgewertet: Trotz
// bestätigter Durchgängigkeit und RPM-abhängiger Spannung am Pin kamen nie zählbare Flanken
// an - der Lüfter liefert vermutlich kein klassisches Puls-Tachosignal. Ohne Oszilloskop
// nicht weiter eingrenzbar, Drehzahlmessung daher fallengelassen.

// Lineare Drosselung der LED-Helligkeit oberhalb dieser Temperatur (fällt zusammen mit
// FAN_CURVE_MAX_TEMP_C - ab hier läuft der Lüfter schon auf 100%, weiteres Ansteigen wird nur
// noch über die LED-Leistung selbst gebremst), bis bei LED_THROTTLE_ZERO_C nichts mehr übrig
// bleibt. Wirkt als Obergrenze auf die eingestellte Helligkeit, hebt sie nie an - kein harter
// Schwellwert/Hysterese nötig, die Rampe selbst verhindert Flattern.
#define LED_THROTTLE_START_C 65.0f
#define LED_THROTTLE_ZERO_C 80.0f

// Akkuspannung über Spannungsteiler - nachgemessen: B0 (ADC1_IN8). Teiler ist einstellbar
// (Trimmer), auf ~3,0V Pin-Spannung bei max. Akkuspannung (~22V) ausgelegt für volle
// ADC-Auflösung. Fein-kalibriert gegen Multimeter (nach vollem EMA-Einschwingen, ~30-40s):
// Anzeige 20,00V vs. gemessen 20,179V -> Korrektur 20,179/20,00 = 1,00895
// -> Faktor 7,83 * 1,00895 = 7,90.
// Bei erneutem Verstellen des Trimmers muss dieser Faktor neu kalibriert werden.
#define PIN_BATTERY_ADC PB0
#define BATTERY_VOLTAGE_DIVIDER_RATIO 7.90f

// Schaltbares "GND" für den Teiler - nachgemessen: C14 (liegt am OSC32_IN-Pad für den
// RTC-Quarz, der auf den meisten Bluepill-Boards unbestückt ist - als GPIO unproblematisch,
// sofern kein 32,768kHz-Quarz auf dem Board bestückt ist). Liegt nur während der Messung
// auf LOW (Teiler geschlossen), sonst hochohmig (Eingang, Teiler offen) - vermeidet die
// sonst dauerhafte Parasitärlast (~2mA bei 10kΩ-Teiler) im Sleep und zwischen Messzyklen.
#define PIN_BATTERY_GND PC14
// RC-Einschwingzeit nach dem Schließen des Teilers, bevor gemessen wird (5x Zeitkonstante,
// worst case ~2,5kΩ Teiterimpedanz * 100nF Puffer-C am ADC-Pin ≈ 250µs -> aufgerundet).
#define BATTERY_GND_SETTLE_TIME_US 2000

// Notabschaltung bei Unterspannung: Der Ferrex-Akku hat laut Recherche keinen internen
// BMS-Zellschutz (nur NTC, kein Balancer/Cutoff) - diese Schwelle ist daher die einzige
// Schutzschicht gegen Tiefentladung, nicht nur ein Backup. 15,0V = 3,0V/Zelle bei 5S.
#define BATTERY_CUTOFF_VOLTAGE 15.0f

// Lineare Drosselung der LED-Helligkeit unterhalb dieser Spannung, bis bei
// BATTERY_CUTOFF_VOLTAGE nichts mehr übrig bleibt. Wirkt als Obergrenze, hebt eine niedrigere
// eingestellte Helligkeit nie an - kein harter Schwellwert/Hysterese nötig, die Rampe selbst
// ist selbstregulierend gegen den Lastabsenkungs-Effekt (IR-Drop bei hohem LED-Strom): sinkt
// die Helligkeit, sinkt auch der Strom, die Spannung erholt sich, statt hart zwischen Ein/Aus
// zu takten.
#define BATTERY_LED_RAMP_START_VOLTAGE 16.5f

// Für die Ladezustands-Prozentanzeige: 100% bei voller Ladeschlussspannung (4,2V/Zelle bei
// 5S), 0% beim Cutoff (BATTERY_CUTOFF_VOLTAGE).
#define BATTERY_FULL_VOLTAGE 21.0f

// DS18B20 auf dem Kühlkörper (OneWire) - nachgemessen: A2.
#define PIN_DS18B20 PA2

// Inkrementalgeber (KY-040-artiges Modul: CLK/DT/SW) - nachgemessen: CLK=A0, DT=A1, SW=A3.
#define PIN_ENC_CLK PA0
#define PIN_ENC_DT  PA1
#define PIN_ENC_SW  PA3
#define ENCODER_BUTTON_DEBOUNCE_MS 30
// Ab dieser Haltedauer zählt ein Tastendruck als "lang" (Aus/Deep-Sleep) statt "kurz"
// (Ein/Turbo-Toggle).
#define ENCODER_LONG_PRESS_MS 600

// Fester Startwert statt EEPROM-Persistenz: Der Spannungswandler zwischen Akku und Board hält
// die Spannung bei Stromverlust lange und bricht dann abrupt weg - zu wenig Vorwarnzeit für
// einen zuverlässigen Notspeichervorgang. Nach jedem Neustart (Akku angeschlossen) gilt daher
// immer dieser Wert als Helligkeit, bis der Nutzer per Encoder nachjustiert.
#define LED_DEFAULT_PERCENT 35
